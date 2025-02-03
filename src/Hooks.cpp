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
            logger::info("{} bashes with {}", actor->GetName(), shield->GetName(), shield->GetWeight());
            staminaCostDmg = (shield->GetWeight() * bashMult) + bashBase;

            logger::info("Cost before: {}", staminaCostDmg);
            BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModPowerAttackStamina, actor, shield, addressof(staminaCostDmg));
            logger::info("Cost after: {}", staminaCostDmg);
        } else if (equip->IsWeapon()) {
            TESObjectWEAP* weapon = equip->As<TESObjectWEAP>();
            logger::info("{} bashes with {}", actor->GetName(), weapon->GetName(), weapon->GetWeight());
            staminaCostDmg = (weapon->GetWeight() * bashMult) + bashBase;

            logger::info("Cost before: {}", staminaCostDmg);
            BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModPowerAttackStamina, actor, weapon, addressof(staminaCostDmg));
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
            staminaCostDmg = (weapon->GetWeight() * swingMult) + swingBase;

            logger::info("Cost before: {}", staminaCostDmg);
            BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModPowerAttackStamina, actor, weapon, addressof(staminaCostDmg));
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