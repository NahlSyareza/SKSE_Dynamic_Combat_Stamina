#include "Hooks.h"

using namespace RE;

bool IsRecoiling(RE::Actor* actor) {
    bool state = false;
    return actor->GetGraphVariableBool("IsRecoiling", state) && state;
}

bool IsRightCasting(RE::Actor* actor) {
    bool state = false;
    return actor->GetGraphVariableBool("IsCastingRight", state) && state;
}

bool IsLeftCasting(RE::Actor* actor) {
    bool state = false;
    return actor->GetGraphVariableBool("IsCastingLeft", state) && state;
}

bool IsDualCasting(RE::Actor* actor) {
    bool state = false;
    return actor->GetGraphVariableBool("IsCastingDual", state) && state;
}

bool IsCasting(RE::Actor* actor) { return IsRightCasting(actor) || IsLeftCasting(actor) || IsDualCasting(actor); }

bool IsDrawing(RE::Actor* actor) { return actor->AsActorState()->GetAttackState() == RE::ATTACK_STATE_ENUM::kBowDrawn; }

bool IsAttacking(RE::Actor* actor) {
    bool state = false;
    return actor->GetGraphVariableBool("IsAttacking", state) && state;
}

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

        if (equip->IsArmor()) {
            // It should be shield
            TESObjectARMO* shield = equip->As<TESObjectARMO>();
            logger::info("{} bashes with {}", actor->GetName(), shield->GetName(), shield->GetWeight());
            staminaCostDmg = (shield->GetWeight() * bashMult) + bashBase;
        } else if (equip->IsWeapon()) {
            TESObjectWEAP* weapon = equip->As<TESObjectWEAP>();
            logger::info("{} bashes with {}", actor->GetName(), weapon->GetName(), weapon->GetWeight());
            staminaCostDmg = (weapon->GetWeight() * bashMult) + bashBase;

            // float copy = staminaCostDmg;
            // logger::info("Before {}", copy * atkData->data.staminaMult);
            // RE::BGSEntryPoint::HandleEntryPoint(RE::BGSEntryPoint::ENTRY_POINTS::kModPowerAttackStamina, actor, objToCheck, std::addressof(copy));
            // logger::info("After {}", copy * atkData->data.staminaMult);
        } else {
            logger::info("{} bashes with {} ???", actor->GetName(), equip->GetName());
        }
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
        }
    }

    if (atkData->data.flags.any(RE::AttackData::AttackFlag::kPowerAttack)) {
        logger::info("{} is power attacking", actor->GetName());
        staminaCostDmg *= powerMult;
    }

    logger::info("Stamina damage {}", staminaCostDmg);
    // _ActionStaminaCost(avOwner, atkData);
    return staminaCostDmg;
}

/*
Issue with the hook above
P.S. (No, it's the fucking godmode)
Either stamina draining or block breaking mechanic
*/

void Hooks::CombatHit::HitImpact(RE::Actor* target, RE::HitData& hitData) {
    Actor* aggressor = hitData.aggressor.get().get();

    /*If there is no target or aggressor, return*/
    if (!target || !aggressor || target->IsDead()) {
        logger::info("Target or aggresor not found");
        _HitImpact(target, hitData);
        return;
    }

    /*
    Is this the answer?
    if (target->IsPlayerRef() && IsRecoiling(target)) {
        float mult = hitData.flags.any(RE::HitData::Flag::kPowerAttack) ? 0.15F : 0.5F;
        TryStagger(target, mult, aggressor);
    }
    */

    /*Only registers if it is a bash*/
    if (hitData.flags.any(RE::HitData::Flag::kBash)) {
        /*Blocked bash do no damage*/
        if (hitData.flags.any(RE::HitData::Flag::kBlocked)) {
            hitData.totalDamage = 0;
            _HitImpact(target, hitData);
            return;
        }

        logger::info("{} bashed {}", aggressor->GetName(), target->GetName());

        float staminaDamage = hitData.totalDamage;

        logger::info("Stamina damage {}", staminaDamage);

        target->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kStamina, -staminaDamage);

        hitData.totalDamage = 0;
    }

    _HitImpact(target, hitData);
}

// Change this with effect-applying restriction
bool Hooks::CombatAction::DoCombatAction(RE::TESActionData* actData) {
    TESObjectREFR* actorRef = actData->source.get();

    if (!actData || !actorRef) {
        return _DoCombatAction(actData);
    }

    Actor* actor = actorRef->As<RE::Actor>();

    if (!actor) {
        return _DoCombatAction(actData);
    }

    float staminaVal = actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);

    const char* aEvent = actData->animEvent.c_str();

    if (strstr(aEvent, "attack")) {
        bool isLeft = strstr(aEvent, "Left");

        const char* attackMsg = isLeft ? "left attack!" : "right attack!";

        logger::info("Bro wants to {}!", attackMsg);

        TESForm* attack = actor->GetEquippedObject(isLeft);

        if (!attack) {
            logger::info("No attack");
            return _DoCombatAction(actData);
        }

        if (staminaVal < attack->GetWeight()) {
            return false;
        }
    }

    if (strstr(aEvent, "bash")) {
        logger::info("Bro wants to bash!");

        RE::TESForm* bash = actor->GetEquippedObject(getEquippedShield(actor));

        if (!bash) {
            logger::info("No bash?");
            return _DoCombatAction(actData);
        }

        if (staminaVal < bash->GetWeight()) {
            return false;
        }
    }

    return _DoCombatAction(actData);
}