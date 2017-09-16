#ifndef RWE_MAINMENUCONTROLLER_H
#define RWE_MAINMENUCONTROLLER_H

#include <memory>
#include <rwe/AudioService.h>
#include <rwe/CursorService.h>
#include <rwe/MainMenuModel.h>
#include <rwe/MainMenuScene.h>
#include <rwe/observable/Subscription.h>
#include <rwe/tdf/SimpleTdfAdapter.h>
#include <rwe/ui/UiFactory.h>
#include <rwe/vfs/AbstractVirtualFileSystem.h>

#include "SceneManager.h"

namespace rwe
{
    class MainMenuController
    {
    private:
        AbstractVirtualFileSystem* vfs;
        SceneManager* sceneManager;
        TdfBlock* allSoundTdf;
        AudioService* audioService;
        TextureService* textureService;
        CursorService* cursor;
        MainMenuModel* model;

        UiFactory uiFactory;
        std::shared_ptr<MainMenuScene> scene;

        AudioService::LoopToken bgmHandle;

        std::vector<std::unique_ptr<Subscription>> subscriptions;

    public:
        MainMenuController(
            AbstractVirtualFileSystem* vfs,
            SceneManager* sceneManager,
            TdfBlock* allSoundTdf,
            AudioService* audioService,
            TextureService* textureService,
            CursorService* cursor,
            MainMenuModel* model);

        ~MainMenuController();

        void goToMainMenu();

        void goToSingleMenu();

        void goToSkirmishMenu();

        void goToPreviousMenu();

        void openMapSelectionDialog();

        void start();

        void exit();

        void message(const std::string& topic, const std::string& message);

        void scrollMessage(const std::string& topic, unsigned int group, const std::string& name, const ScrollPositionMessage& m);

        void scrollUpMessage(const std::string& topic, unsigned int group, const std::string& name);

        void scrollDownMessage(const std::string& topic, unsigned int group, const std::string& name);

        void setCandidateSelectedMap(const std::string& mapName);

        void clearCandidateSelectedMap();

        void commitSelectedMap();

        void resetCandidateSelectedMap();

        void togglePlayer(int playerIndex);

        void incrementPlayerMetal(int playerIndex);

        void decrementPlayerMetal(int playerIndex);

        void incrementPlayerEnergy(int playerIndex);

        void decrementPlayerEnergy(int playerIndex);

        void togglePlayerSide(int playerIndex);

        void cyclePlayerColor(int playerIndex);

        void reverseCyclePlayerColor(int playerIndex);

        void cyclePlayerTeam(int playerIndex);

        void startGame();
    };
}


#endif