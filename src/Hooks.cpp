#include "Hooks.h"

using namespace RE;
using namespace std;

/**
 * Doesn't work with god mode
 */

float Hooks::CombatStamina::ActionStaminaCost(RE::ActorValueOwner* avOwner, RE::BGSAttackData* atkData) {
    RE::Actor* actor = skyrim_cast<RE::Actor*>(avOwner);

    if (!actor) {
        logger::info("Actor not found!");
        return _ActionStaminaCost(avOwner, atkData);
    }

    logger::info("{} detected", actor->GetName());

    auto gameSettings = RE::GameSettingCollection::GetSingleton();

    // How can I get custom GMST pls TwT
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
    float perkMult = 1.0F;

    float powerMult = 5.0F;

    const char* isLeft = atkData->IsLeftAttack() ? "Left" : "Right";
    logger::info("{} attack registered", isLeft);

    bool hasShield = getEquippedShield(actor);

    // if (hasShield) {
    //     logger::info("Is equipped with shield!");
    // } else {
    //     logger::info("Is not equipped with shield!");
    // }

    if (atkData->data.flags.any(RE::AttackData::AttackFlag::kBashAttack)) {
        logger::info("Start bashing");

        /*If bashing*/

        /**
         * Potential error by type casting using ->As<>()
         */
        TESForm* equip = actor->GetEquippedObject(hasShield);

        if (!equip) {
            logger::info("Can't detect bash object");
            return _ActionStaminaCost(avOwner, atkData);
        }

        if (equip->IsArmor()) {
            // It should be shield
            TESObjectARMO* shield = equip->As<TESObjectARMO>();
            float av = actor->AsActorValueOwner()->GetActorValue(ActorValue::kBlock);
            float eff = av / 2;
            logger::info("Efficiency {}% (Block skill {})", floor(eff), av);
            logger::info("Bashes with {}", shield->GetName());
            staminaCostDmg = (shield->GetWeight() * bashMult) + bashBase;
            staminaCostDmg *= (1 - (floor(eff) / 100));

            // Potential error
            BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModPowerAttackStamina, actor, shield, &perkMult);
            logger::info("Perk mult: {}", perkMult);

        } else if (equip->IsWeapon()) {
            // It should be weapon
            TESObjectWEAP* weapon = equip->As<TESObjectWEAP>();
            logger::info("Bashes with {}", weapon->GetName());
            float av = 0;
            float eff = 1;
            if (weapon->IsOneHandedAxe() || weapon->IsOneHandedSword() || weapon->IsOneHandedMace() || weapon->IsOneHandedDagger()) {
                av = actor->AsActorValueOwner()->GetActorValue(ActorValue::kOneHanded);
                eff = av / 2;
                logger::info("Efficiency {}% (One-Handed skill {})", floor(eff), av);
            } else if (weapon->IsTwoHandedAxe() || weapon->IsTwoHandedSword()) {
                av = actor->AsActorValueOwner()->GetActorValue(ActorValue::kTwoHanded);
                eff = av / 2;
                logger::info("Efficiency {}% (Two-Handed skill {})", floor(eff), av);
            } else {
                logger::info("You shouldn't be here!");
            }

            staminaCostDmg = (weapon->GetWeight() * bashMult) + bashBase;
            staminaCostDmg *= (1 - (floor(eff) / 100));

            // Potential error
            BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModPowerAttackStamina, actor, weapon, &perkMult);
            logger::info("Perk mult: {}", perkMult);
        } else {
            logger::info("Bashes with {} ???", equip->GetName());
        }

        if (atkData->data.flags.any(RE::AttackData::AttackFlag::kPowerAttack)) {
            logger::info("Is Power");

            logger::info("Initial cost: {}", staminaCostDmg);
            logger::info("Final cost: {}", staminaCostDmg * powerMult * perkMult * atkData->data.staminaMult);

            return staminaCostDmg * powerMult * perkMult * atkData->data.staminaMult;
        }

        logger::info("Initial cost: {}", staminaCostDmg);
        logger::info("Final cost: {}", staminaCostDmg * atkData->data.staminaMult);

        return staminaCostDmg * atkData->data.staminaMult;
    } else {
        logger::info("Start swinging");
        /**
         * If is attacking (Can't find a flag for a normal swinging. Flag kNone doesn't work as well as the others, but
         * hey there are only swings and bashes anyway. Bow doesn't fire this event so don't worry)
         */

        // Can probably cause error
        InventoryEntryData* equip = actor->GetAttackingWeapon();

        if (!equip) {
            logger::info("Can't detect swing object");
            return _ActionStaminaCost(avOwner, atkData);
        }

        TESObjectWEAP* weapon = equip->GetObject()->As<RE::TESObjectWEAP>();
        logger::info("Swings with {}", weapon->GetName());

        float av = 0;
        float eff = 1;
        if (weapon->IsOneHandedAxe() || weapon->IsOneHandedSword() || weapon->IsOneHandedMace() || weapon->IsOneHandedDagger()) {
            av = actor->AsActorValueOwner()->GetActorValue(ActorValue::kOneHanded);
            eff = av / 2;
            logger::info("Efficiency {}% (One-Handed skill {})", floor(eff), av);
        } else if (weapon->IsTwoHandedAxe() || weapon->IsTwoHandedSword()) {
            av = actor->AsActorValueOwner()->GetActorValue(ActorValue::kTwoHanded);
            eff = av / 2;
            logger::info("Efficiency {}% (Two-Handed skill {})", floor(eff), av);
        } else {
            logger::info("You shouldn't be here!");
        }

        float actualWeight = weapon->GetWeight() < 1 ? 5 : weapon->GetWeight();
        staminaCostDmg = (actualWeight * swingMult) + swingBase;
        staminaCostDmg *= (1 - (floor(eff) / 100));

        BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModPowerAttackStamina, actor, weapon, &perkMult);
        logger::info("Perk mult: {}", perkMult);

        if (atkData->data.flags.any(RE::AttackData::AttackFlag::kPowerAttack)) {
            logger::info("Is Power", actor->GetName());
            // staminaCostDmg *= powerMult;

            logger::info("Initial cost: {}", staminaCostDmg);
            logger::info("Final cost: {}", staminaCostDmg * powerMult * perkMult * atkData->data.staminaMult);

            return staminaCostDmg * powerMult * perkMult * atkData->data.staminaMult;
        }

        logger::info("Initial cost: {}", staminaCostDmg);
        logger::info("Final cost: {}", staminaCostDmg * atkData->data.staminaMult);

        return staminaCostDmg * atkData->data.staminaMult;
    }

    return _ActionStaminaCost(avOwner, atkData);
}

// Currently unused
void Hooks::CombatHit::HitImpact(RE::Actor* target, RE::HitData& hitData) { _HitImpact(target, hitData); }
bool Hooks::CombatAction::DoCombatAction(RE::TESActionData* actData) { return _DoCombatAction(actData); }