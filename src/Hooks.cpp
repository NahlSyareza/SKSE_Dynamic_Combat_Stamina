#include "Hooks.h"

using namespace RE;
using namespace std;

/**
 * Doesn't work with god mode
 * Currently not supporting stamina reducing perks
 */
float Hooks::CombatStamina::ActionStaminaCost(RE::ActorValueOwner* avOwner, RE::BGSAttackData* atkData) {
    RE::Actor* actor = skyrim_cast<RE::Actor*>(avOwner);

    if (!actor) {
        logger::info("Actor not found!");
        return _ActionStaminaCost(avOwner, atkData);
    }

    RE::GameSettingCollection* gameSettings = RE::GameSettingCollection::GetSingleton();

    // How about using these values instead yeaa?
    RE::Setting* normalStaminaMult = gameSettings->GetSetting("fDCSNormalStaminaMult");
    RE::Setting* normalStaminaBase = gameSettings->GetSetting("fDCSNormalStaminaBase");
    RE::Setting* powerStaminaMult = gameSettings->GetSetting("fDCSPowerStaminaMult");
    RE::Setting* powerStaminaBase = gameSettings->GetSetting("fDCSPowerStaminaBase");

    if (normalStaminaMult) {
        logger::info("Normal mult: {}", normalStaminaMult->GetFloat());
    } else {
        logger::info("Could not find GMST");
    }

    float staminaCostDmg = 5.0F;

    float swingMult = 1.0F;
    float swingBase = 0.0F;
    float bashMult = 1.0F;
    float bashBase = 0.0F;

    float powerMult = 5.0F;

    const char* isLeft = atkData->IsLeftAttack() ? "Left" : "Right";
    logger::info("{} attack registered", isLeft);

    bool hasShield = getEquippedShield(actor);

    if (hasShield) {
        logger::info("Actor is equipped with shield!");
    } else {
        logger::info("Actor is not equipped with shield!");
    }

    if (atkData->data.flags.any(RE::AttackData::AttackFlag::kBashAttack)) {
        logger::info("Start bashing");

        /*If bashing*/

        /**
         * Potential error by type casting using ->As<>()
         */
        TESForm* equip = actor->GetEquippedObject(hasShield);

        if (!equip) {
            logger::info("Can't detect equipped object");
            return _ActionStaminaCost(avOwner, atkData);
        }

        if (equip->IsArmor()) {
            // It should be shield
            TESObjectARMO* shield = equip->As<TESObjectARMO>();
            float av = actor->AsActorValueOwner()->GetActorValue(ActorValue::kBlock);
            int eff = av / 2 < 0 ? 1 : av / 2;
            logger::info("Efficiency {}% (Block skill {})", eff, av);
            logger::info("{} bashes with {}", actor->GetName(), shield->GetName(), shield->GetWeight(), av, eff);
            staminaCostDmg = (shield->GetWeight() * bashMult) + bashBase;
            staminaCostDmg /= eff;

            logger::info("Cost before: {}", staminaCostDmg);
            BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModPowerAttackStamina, actor, shield, &staminaCostDmg);
            logger::info("Cost after: {}", staminaCostDmg);
        } else if (equip->IsWeapon()) {
            TESObjectWEAP* weapon = equip->As<TESObjectWEAP>();
            logger::info("{} bashes with {}", actor->GetName(), weapon->GetName(), weapon->GetWeight());

            float av = 0;
            int eff = 1;
            if (weapon->IsOneHandedAxe() || weapon->IsOneHandedSword() || weapon->IsOneHandedMace() || weapon->IsOneHandedDagger()) {
                av = actor->AsActorValueOwner()->GetActorValue(ActorValue::kOneHanded);
                eff = av / 2 < 0 ? 1 : av / 2;
                logger::info("Efficiency {}% (One-Handed skill {})", eff, av);
            } else if (weapon->IsTwoHandedAxe() || weapon->IsTwoHandedSword()) {
                av = actor->AsActorValueOwner()->GetActorValue(ActorValue::kTwoHanded);
                eff = av / 2 < 0 ? 1 : av / 2;
                logger::info("Efficiency {}% (Two-Handed skill {})", eff, av);
            } else {
                logger::info("You shouldn't be here!");
            }

            staminaCostDmg = (weapon->GetWeight() * bashMult) + bashBase;
            staminaCostDmg /= eff;

            logger::info("Cost before: {}", staminaCostDmg);
            BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModPowerAttackStamina, actor, weapon, &staminaCostDmg);
            logger::info("Cost after: {}", staminaCostDmg);
        } else {
            logger::info("{} bashes with {} ???", actor->GetName(), equip->GetName());
        }

        if (atkData->data.flags.any(RE::AttackData::AttackFlag::kPowerAttack)) {
            logger::info("{} is power attacking", actor->GetName());
            staminaCostDmg *= powerMult;
        }

        return staminaCostDmg * atkData->data.staminaMult;
    } else {
        logger::info("Start swinging");
        /*
        If is attacking (Can't find a flag for a normal swinging. Flag kNone doesn't work as well as the others, but
        hey there are only swings and bashes anyway. Bow doesn't fire this event so don't worry)
        */
        if (actor->GetAttackingWeapon()) {
            TESObjectWEAP* weapon = actor->GetAttackingWeapon()->GetObject()->As<RE::TESObjectWEAP>();
            logger::info("{} swings with {}", actor->GetName(), weapon->GetName());

            float av = 0;
            int eff = 1;
            if (weapon->IsOneHandedAxe() || weapon->IsOneHandedSword() || weapon->IsOneHandedMace() || weapon->IsOneHandedDagger()) {
                av = actor->AsActorValueOwner()->GetActorValue(ActorValue::kOneHanded);
                eff = av / 2 < 0 ? 1 : av / 2;
                logger::info("Efficiency {}% (One-Handed skill {})", eff, av);
            } else if (weapon->IsTwoHandedAxe() || weapon->IsTwoHandedSword()) {
                av = actor->AsActorValueOwner()->GetActorValue(ActorValue::kTwoHanded);
                eff = av / 2 < 0 ? 1 : av / 2;
                logger::info("Efficiency {}% (Two-Handed skill {})", eff, av);
            } else {
                logger::info("You shouldn't be here!");
            }

            float actualWeight = weapon->GetWeight() < 1 ? 5 : weapon->GetWeight();
            staminaCostDmg = (actualWeight * swingMult) + swingBase;
            staminaCostDmg /= eff;

            logger::info("Cost before: {}", staminaCostDmg);
            BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModPowerAttackStamina, actor, weapon, &staminaCostDmg);
            logger::info("Cost after: {}", staminaCostDmg);
        }

        if (atkData->data.flags.any(RE::AttackData::AttackFlag::kPowerAttack)) {
            logger::info("{} is power attacking", actor->GetName());
            staminaCostDmg *= powerMult;
        }

        return staminaCostDmg * atkData->data.staminaMult;
    }

    return _ActionStaminaCost(avOwner, atkData);
}

/*
Issue with the hook above
P.S. (No, it's the fucking godmode)
Either stamina draining or block breaking mechanic
*/

void Hooks::CombatHit::HitImpact(RE::Actor* target, RE::HitData& hitData) { _HitImpact(target, hitData); }

// Change this with effect-applying restriction
bool Hooks::CombatAction::DoCombatAction(RE::TESActionData* actData) { return _DoCombatAction(actData); }