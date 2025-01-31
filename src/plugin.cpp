#include "Hooks.h"
#include "Logger.h"

class EventSink : public RE::BSTEventSink<SKSE::ActionEvent> {
public:
    RE::BSEventNotifyControl ProcessEvent(const SKSE::ActionEvent* event, RE::BSTEventSource<SKSE::ActionEvent>*) {
        // if (event->actor && event->actor->IsPlayerRef()) {
        //     auto* player = event->actor;

        //     if (event->type.any(SKSE::ActionEvent::Type::kBowDraw)) {
        //         logger::info("{} is drawing!", player->GetName());

        //         auto* weap = player->GetEquippedObject(true);

        //         if (event->sourceForm) {
        //             logger::info("Source form is {}", event->sourceForm->GetName());
        //         }

        //         if (weap) {
        //             logger::info("Using {}", weap->GetName());
        //         } else {
        //             logger::info("Using nothing!");
        //         }

        //         return RE::BSEventNotifyControl::kContinue;
        //     }
        // }

        return RE::BSEventNotifyControl::kContinue;
    }
};

void OnMessage(SKSE::MessagingInterface::Message* message) {
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        // Start
    }
    if (message->type == SKSE::MessagingInterface::kNewGame || message->type == SKSE::MessagingInterface::kPostLoadGame) {
        // Post-load
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);

    SetupLog();
    logger::info("Plugin loaded");

    EventSink* eventSink = new EventSink();

    SKSE::GetActionEventSource()->AddEventSink(eventSink);

    Hooks::Install();

    return true;
}