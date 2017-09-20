#include "TextureService.h"
#include <rwe/pcx.h>
#include <rwe/Gaf.h>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <rwe/tnt/TntArchive.h>

namespace rwe
{
    class BufferGafAdapter : public GafReaderAdapter
    {
    private:
        GraphicsContext* graphics;
        const ColorPalette* palette;
        std::vector<Color> buffer;
        GafFrameData currentFrameHeader;

        SpriteSeries spriteSeries;

    public:
        explicit BufferGafAdapter(GraphicsContext* graphics, const ColorPalette* palette) : graphics(graphics), palette(palette), currentFrameHeader() {}

        void beginFrame(const GafFrameData& header) override
        {
            buffer.clear();
            buffer.resize(header.width * header.height);
            std::fill(buffer.begin(), buffer.end(), Color::Transparent);
            currentFrameHeader = header;
        }

        void frameLayer(const LayerData& data) override
        {
            for (std::size_t y = 0; y < data.height; ++y)
            {
                for (std::size_t x = 0; x < data.width; ++x)
                {
                    auto outPosX = static_cast<int>(x) - (data.x - currentFrameHeader.posX);
                    auto outPosY = static_cast<int>(y) - (data.y - currentFrameHeader.posY);

                    if (outPosX < 0 || outPosX >= currentFrameHeader.width || outPosY < 0 || outPosY >= currentFrameHeader.height)
                    {
                        throw std::runtime_error("frame coordinate out of bounds");
                    }

                    auto colorIndex = static_cast<unsigned char>(data.data[(y * data.width) + x]);
                    if (colorIndex == data.transparencyKey)
                    {
                        continue;
                    }

                    buffer[(outPosY * currentFrameHeader.width) + outPosX] = (*palette)[colorIndex];
                }
            }
        }

        void endFrame() override
        {
            auto handle = graphics->createTexture(currentFrameHeader.width, currentFrameHeader.height, buffer);

            Sprite sprite(
                Rectangle2f::fromTopLeft(
                    -currentFrameHeader.posX,
                    -currentFrameHeader.posY,
                    currentFrameHeader.width,
                    currentFrameHeader.height),
                handle);

            spriteSeries.sprites.push_back(std::move(sprite));
        }

        SpriteSeries extractSpriteSeries()
        {
            return std::move(spriteSeries);
        }
    };

    TextureService::TextureService(GraphicsContext* graphics, AbstractVirtualFileSystem* fileSystem, const ColorPalette* palette)
        : graphics(graphics), fileSystem(fileSystem), palette(palette)
    {
        auto handle = graphics->createColorTexture(Color(255, 0, 255));

        auto series = std::make_shared<SpriteSeries>();
        series->sprites.emplace_back(Rectangle2f(0.5f, 0.5f, 0.5f, 0.5f), handle);
        defaultSpriteSeries = std::move(series);
    }

    boost::optional<std::shared_ptr<SpriteSeries>> TextureService::getGafEntryInternal(const std::string& gafName, const std::string& entryName)
    {
        auto key = gafName + "/" + entryName;
        auto it = animCache.find(key);
        if (it != animCache.end())
        {
            return it->second;
        }

        auto gafBytes = fileSystem->readFile(gafName);
        if (!gafBytes)
        {
            return boost::none;
        }

        boost::interprocess::bufferstream gafStream(gafBytes->data(), gafBytes->size());
        GafArchive gafArchive(&gafStream);

        auto gafEntry = gafArchive.findEntry(entryName);
        if (!gafEntry)
        {
            return boost::none;
        }

        BufferGafAdapter adapter(graphics, palette);
        gafArchive.extract(*gafEntry, adapter);
        auto ptr = std::make_shared<SpriteSeries>(adapter.extractSpriteSeries());
        animCache[key] = ptr;
        return ptr;
    }

    std::shared_ptr<SpriteSeries> TextureService::getGafEntry(const std::string& gafName, const std::string& entryName)
    {
        auto entry = getGafEntryInternal(gafName, entryName);
        if (!entry)
        {
            throw std::runtime_error("Failed to load GAF entry");
        }

        return *entry;
    }

    boost::optional<std::shared_ptr<SpriteSeries>>
    TextureService::getGuiTexture(const std::string& guiName, const std::string& graphicName)
    {
        auto entry = getGafEntryInternal("anims/" + guiName + ".gaf", graphicName);
        if (entry)
        {
            return entry;
        }

        entry = getGafEntryInternal("anims/commongui.gaf", graphicName);
        if (entry)
        {
            return entry;
        }

        return boost::none;
    }

    SharedTextureHandle TextureService::getBitmap(const std::string& bitmapName)
    {
        auto info = getBitmapInternal(bitmapName);
        return info.handle;
    }

    std::shared_ptr<SpriteSeries> TextureService::getDefaultSpriteSeries()
    {
        return defaultSpriteSeries;
    }

    SharedTextureHandle TextureService::getDefaultTexture()
    {
        return defaultSpriteSeries->sprites[0].texture.texture;
    }

    Sprite TextureService::getBitmapRegion(const std::string& bitmapName, int x, int y, int width, int height)
    {
        auto bitmap = getBitmapInternal(bitmapName);
        auto region = Rectangle2f::fromTopLeft(
            static_cast<float>(x) / static_cast<float>(bitmap.width),
            static_cast<float>(y) / static_cast<float>(bitmap.height),
            static_cast<float>(width) / static_cast<float>(bitmap.width),
            static_cast<float>(height) / static_cast<float>(bitmap.height));
        Sprite s(Rectangle2f::fromTopLeft(x, y, width, height), bitmap.handle, region);
        return s;
    }

    TextureService::TextureInfo TextureService::getBitmapInternal(const std::string& bitmapName)
    {
        auto it = bitmapCache.find(bitmapName);
        if (it != bitmapCache.end())
        {
            return it->second;
        }

        auto entry = fileSystem->readFile("bitmaps/" + bitmapName + ".pcx");
        if (!entry)
        {
            throw std::runtime_error("bitmap not found");
        }

        PcxDecoder decoder(entry->begin().base(), entry->end().base());

        auto decodedData = decoder.decodeImage();
        auto palette = decoder.decodePalette();

        auto width = decoder.getWidth();
        auto height = decoder.getHeight();

        std::vector<Color> buffer(decodedData.size());
        for (std::size_t i = 0; i < decodedData.size(); ++i)
        {
            auto paletteIndex = static_cast<unsigned char>(decodedData[i]);
            assert(paletteIndex < palette.size());
            buffer[i] = palette[paletteIndex].toColor();
        }

        auto handle = graphics->createTexture(width, height, buffer);
        TextureInfo info(width, height, handle);
        bitmapCache[bitmapName] = info;
        return info;
    }

    Sprite TextureService::getMinimap(const std::string& mapName)
    {
        auto it = minimapCache.find(mapName);
        if (it != minimapCache.end())
        {
            return it->second;
        }

        auto tntData = fileSystem->readFile("maps/" + mapName + ".tnt");
        if (!tntData)
        {
            throw std::runtime_error("map tnt not found!");
        }

        boost::interprocess::bufferstream tntStream(tntData->data(), tntData->size());
        TntArchive tnt(&tntStream);
        auto minimap = tnt.readMinimap();

        std::vector<Color> rgbMinimap;
        std::transform(minimap.data.begin(), minimap.data.end(), std::back_inserter(rgbMinimap), [p = palette](unsigned char pixel) {
            assert(pixel >= 0 && pixel <= 255);
            return (*p)[pixel];
        });

        auto texture = graphics->createTexture(minimap.width, minimap.height, rgbMinimap);
        Sprite sprite(Rectangle2f::fromTopLeft(0.0f, 0.0f, minimap.width, minimap.height), texture);

        minimapCache.insert({mapName, sprite});

        return sprite;
    }

    TextureService::TextureInfo::TextureInfo(unsigned int width, unsigned int height, const SharedTextureHandle& handle)
        : width(width), height(height), handle(handle)
    {
    }
}
