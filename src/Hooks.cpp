#include "Hooks.h"

#include "mini/ini.h"

using namespace RE;
// using namespace std;

/**
 * Doesn't work with god mode
 */

float Hooks::CombatStamina::ActionStaminaCost(ActorValueOwner* avOwner, BGSAttackData* atkData) {
    Actor* actor = skyrim_cast<Actor*>(avOwner);

    if (!actor) {
        logger::info("Actor not found!");
        return _ActionStaminaCost(avOwner, atkData);
    }

    logger::info("{} detected", actor->GetName());

    auto gameSettings = GameSettingCollection::GetSingleton();

    // How can I get custom GMST pls TwT
    Setting* normalStaminaMult = gameSettings->GetSetting("fDCSNormalStaminaMult");
    Setting* normalStaminaBase = gameSettings->GetSetting("fDCSNormalStaminaBase");
    Setting* powerStaminaMult = gameSettings->GetSetting("fDCSPowerStaminaMult");
    Setting* powerStaminaBase = gameSettings->GetSetting("fDCSPowerStaminaBase");

    if (normalStaminaMult) {
        logger::info("Normal mult: {}", normalStaminaMult->GetFloat());
    } else {
        logger::info("Could not find GMST");
    }

    float costBase = 1.0F;

    float swingMult = 1.0F;
    float swingBase = 0.0F;
    float bashMult = 1.0F;
    float bashBase = 0.0F;
    float perkMult = 1.0F;

    float powerMult = 5.0F;

    // const char* isLeft = atkData->IsLeftAttack() ? "Left" : "Right";
    // logger::info("{} attack registered", isLeft);

    bool hasShield = Hooks::getEquippedShield(actor);

    // if (hasShield) {
    //     logger::info("Is equipped with shield!");
    // } else {
    //     logger::info("Is not equipped with shield!");
    // }

    if (atkData->data.flags.any(AttackData::AttackFlag::kBashAttack)) {
        logger::info("Start bashing");

        /*If bashing*/

        /**
         * Potential error by type casting using ->As<>()
         */
        TESForm* equip = actor->GetEquippedObject(hasShield);

        if (!equip) {
            logger::info("Can't detect bash object");
            return costBase * atkData->data.staminaMult;
        }

        if (equip->IsArmor()) {
            // It should be shield
            TESObjectARMO* shield = equip->As<TESObjectARMO>();
            float av = actor->AsActorValueOwner()->GetActorValue(ActorValue::kBlock);
            float eff = av / 2;
            logger::info("Efficiency {}% (Block skill {})", floor(eff), av);
            logger::info("Bashes with {}", shield->GetName());
            costBase = (shield->GetWeight() * bashMult) + bashBase;
            costBase *= 1 - (floor(eff) / 100);

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
                logger::info("Efficiency {}% (One-Handed skill {})", floor(av / 2), av);
            } else if (weapon->IsTwoHandedAxe() || weapon->IsTwoHandedSword()) {
                av = actor->AsActorValueOwner()->GetActorValue(ActorValue::kTwoHanded);
                logger::info("Efficiency {}% (Two-Handed skill {})", floor(av / 2), av);
            } else if (weapon->IsCrossbow() || weapon->IsBow()) {
                av = actor->AsActorValueOwner()->GetActorValue(ActorValue::kArchery);
                logger::info("Efficiency {}% (Archery skill {})", floor(av / 2), av);
            } else {
                logger::info("You shouldn't be here!");
            }

            eff = av / 2;

            costBase = (weapon->GetWeight() * bashMult) + bashBase;
            costBase *= 1 - (floor(eff) / 100);

            // Potential error
            BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModPowerAttackStamina, actor, weapon, &perkMult);
            logger::info("Perk mult: {}", perkMult);
        } else {
            logger::info("Bashes with {} ???", equip->GetName());
        }

        if (atkData->data.flags.any(AttackData::AttackFlag::kPowerAttack)) {
            logger::info("Is Power");

            logger::info("Initial cost: {}", costBase);
            logger::info("Final cost: {}", costBase * powerMult * perkMult * atkData->data.staminaMult);

            return costBase * powerMult * perkMult * atkData->data.staminaMult;
        }

        logger::info("Initial cost: {}", costBase);
        logger::info("Final cost: {}", costBase * atkData->data.staminaMult);

        return costBase * atkData->data.staminaMult;
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
            return costBase * atkData->data.staminaMult;
        }

        TESObjectWEAP* weapon = equip->GetObject()->As<TESObjectWEAP>();
        logger::info("Swings with {}", weapon->GetName());

        float av = 0;
        float eff = 1;
        if (weapon->IsOneHandedAxe() || weapon->IsOneHandedSword() || weapon->IsOneHandedMace() || weapon->IsOneHandedDagger()) {
            av = actor->AsActorValueOwner()->GetActorValue(ActorValue::kOneHanded);
            logger::info("Efficiency {}% (One-Handed skill {})", floor(av / 2), av);
        } else if (weapon->IsTwoHandedAxe() || weapon->IsTwoHandedSword()) {
            av = actor->AsActorValueOwner()->GetActorValue(ActorValue::kTwoHanded);
            logger::info("Efficiency {}% (Two-Handed skill {})", floor(av / 2), av);
        } else {
            logger::info("You shouldn't be here!");
        }

        eff = av / 2;

        float actualWeight = weapon->GetWeight() < 1 ? costBase : weapon->GetWeight();
        costBase = (actualWeight * swingMult) + swingBase;
        costBase *= 1 - (floor(eff) / 100);

        BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModPowerAttackStamina, actor, weapon, &perkMult);
        logger::info("Perk mult: {}", perkMult);

        if (atkData->data.flags.any(AttackData::AttackFlag::kPowerAttack)) {
            logger::info("Is Power", actor->GetName());
            // costBase *= powerMult;

            logger::info("Initial cost: {}", costBase);
            logger::info("Final cost: {}", costBase * powerMult * perkMult * atkData->data.staminaMult);

            return costBase * powerMult * perkMult * atkData->data.staminaMult;
        }

        logger::info("Initial cost: {}", costBase);
        logger::info("Final cost: {}", costBase * atkData->data.staminaMult);

        return costBase * atkData->data.staminaMult;
    }

    return _ActionStaminaCost(avOwner, atkData);
}

float calculateEfficiency(Actor* actor, ActorValue av) {
    float av = actor->AsActorValueOwner()->GetActorValue(av);
    float eff = 0.0F;

    eff = av / 2;

    return 1 - (floor(eff) / 100);
}

// Currently unused
void Hooks::CombatHit::HitImpact(Actor* target, HitData& hitData) { _HitImpact(target, hitData); }

bool Hooks::CombatAction::DoCombatAction(TESActionData* actData) {
    Actor* actor = actData->source.get()->As<Actor>();

    if (!actor) {
        return _DoCombatAction(actData);
    }

    float stamina = actor->AsActorValueOwner()->GetActorValue(ActorValue::kStamina);
    const char* event = actData->animEvent.c_str();

    if (strstr(event, "attack") && stamina <= 0) {
        logger::info("{} tries to attack, but have no stamina!", actor->GetName());

        return false;
    }

    return _DoCombatAction(actData);
}