#include "Controller.h"

#include <rwe/gui.h>
#include <rwe/UiPanelScene.h>

#include <stdexcept>

namespace rwe
{
    void Controller::goToMainMenu()
    {
        auto mainMenuGuiRaw = vfs->readFile("guis/MAINMENU.GUI");
        if (!mainMenuGuiRaw)
        {
            throw std::runtime_error("Couldn't read main menu GUI");
        }

        std::string gui(mainMenuGuiRaw->data(), mainMenuGuiRaw->size());
        auto parsedGui = parseGui(parseTdfFromString(gui));
        if (!parsedGui)
        {
            throw std::runtime_error("Failed to parse GUI file");
        }

        auto panel = uiFactory.panelFromGuiFile("MAINMENU", *parsedGui);
        auto scene = std::make_unique<UiPanelScene>(std::move(panel));

        sceneManager->setNextScene(std::move(scene));
    }

    void Controller::startBGM()
    {
        auto allSoundBgm = allSoundTdf->findBlock("BGM");
        if (!allSoundBgm)
        {
            return;
        }

        auto soundName = allSoundBgm->findValue("sound");
        if (!soundName)
        {
            return;
        }


        auto sound = audioService->loadSound(*soundName);
        if (!sound)
        {
            return;
        }

        auto playingSound = bgmHandle.getSound();
        if (playingSound && *playingSound == *sound)
        {
            return;
        }

        bgmHandle = audioService->loopSound(*sound);
    }

    void Controller::start()
    {
        goToMainMenu();
        startBGM();
        sceneManager->execute();
    }

    Controller::Controller(
        AbstractVirtualFileSystem* vfs,
        SceneManager* sceneManager,
        TdfBlock* allSoundTdf,
        AudioService* audioService,
        TextureService* textureService)
            : vfs(vfs),
              sceneManager(sceneManager),
              allSoundTdf(allSoundTdf),
              audioService(audioService),
              textureService(textureService),
              uiFactory(textureService, audioService, allSoundTdf, this)
        {}

    void Controller::exit()
    {
        sceneManager->requestExit();
    }

    void Controller::message(const std::string& topic, const std::string& message)
    {
        if (topic == "MAINMENU" && message == "EXIT")
        {
            exit();
        }
    }
}