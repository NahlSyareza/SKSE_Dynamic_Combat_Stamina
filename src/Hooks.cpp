#include "Hooks.h"

#include "mini/ini.h"

using namespace RE;

/**
 * Doesn't work with god mode
 */

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

bool IsAttacking(RE::Actor* actor) {
    bool state = false;
    return actor->GetGraphVariableBool("IsAttacking", state) && state;
}

bool IsBlocking(RE::Actor* actor) {
    bool state = false;
    return actor->GetGraphVariableBool("Isblocking", state) && state;
}

bool IsStaggering(RE::Actor* actor) {
    bool state = false;
    return actor->GetGraphVariableBool("IsStaggering", state) && state;
}

float Hooks::CombatStamina::ActionStaminaCost(ActorValueOwner* avOwner, BGSAttackData* atkData) {
    Actor* actor = skyrim_cast<Actor*>(avOwner);

    if (!actor) {
        logger::info("Actor not found!");
        return _ActionStaminaCost(avOwner, atkData);
    }

    ATTACK_STATE_ENUM att_st = actor->AsActorState()->GetAttackState();

    switch (att_st) {
        case ATTACK_STATE_ENUM::kBash:
            logger::info("{} bash enum detected!", actor->GetName());
            break;

        case ATTACK_STATE_ENUM::kHit:
            logger::info("{} hit enum detected!", actor->GetName());
            break;

        default:
            logger::info("{} enum not detected!", actor->GetName());
            break;
    }

    auto mini = mINI::GetIniFile("Data/SKSE/Plugins/DCS-DynamicCombatStamina.ini");

    float testValue = mINI::GetIniFloat(mini, "Test", "value");

    logger::info("INI testing value: {}", testValue);

    logger::info("{} detected", actor->GetName());

    float costBase = mINI::GetIniFloat(mini, "Configurations", "costBase");

    // Not configurable cuz it's not meant to be
    float perkMult = 1;

    float powerMult = mINI::GetIniFloat(mini, "Configurations", "powerMult");

    bool hasShield = Hooks::getEquippedShield(actor);

    // if (hasShield) {
    //     logger::info("Is equipped with shield!");
    // } else {
    //     logger::info("Is not equipped with shield!");
    // }

    if (atkData->data.flags.any(AttackData::AttackFlag::kBashAttack)) {
        float bashBase = mINI::GetIniFloat(mini, "Configurations", "bashBase");
        float bashMult = mINI::GetIniFloat(mini, "Configurations", "bashMult");

        logger::info("Start bashing");

        /*If bashing*/

        TESForm* equip = actor->GetEquippedObject(hasShield);

        if (!equip) {
            logger::info("Can't detect bash object");
            return costBase * atkData->data.staminaMult;
        }

        float skillEfficiency = 0;
        if (equip->IsArmor()) {
            // It should be shield
            TESObjectARMO* shield = equip->As<TESObjectARMO>();

            logger::info("Bashes with {} | Weight {}", shield->GetName(), shield->GetWeight());

            float actualCost = shield->GetWeight() < 1 ? 1 : shield->GetWeight();
            calculateSkillEfficiency(actor, ActorValue::kBlock, &skillEfficiency);
            logger::info("Efficiency {}% (Block skill {})", (1 - skillEfficiency) * 100, actor->AsActorValueOwner()->GetActorValue(ActorValue::kBlock));
            logger::info("Bashes with {}", shield->GetName());
            // costBase = ((shield->GetWeight() * bashMult) + bashBase) * skillEfficiency;
            calculateStaminaCost(actualCost, bashBase, bashMult, skillEfficiency, &costBase);

            // Potential error
            BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModPowerAttackStamina, actor, shield, &perkMult);
            logger::info("Perk mult: {}", perkMult);

        } else if (equip->IsWeapon()) {
            // It should be weapon
            TESObjectWEAP* weapon = equip->As<TESObjectWEAP>();
            logger::info("Bashes with {} | Weight {}", weapon->GetName(), weapon->GetWeight());

            float actualCost = weapon->GetWeight() < 1 ? costBase : weapon->GetWeight();
            if (weapon->IsOneHandedAxe() || weapon->IsOneHandedSword() || weapon->IsOneHandedMace() || weapon->IsOneHandedDagger()) {
                calculateSkillEfficiency(actor, ActorValue::kOneHanded, &skillEfficiency);
                logger::info("Efficiency {}% (One-Handed skill {})", (1 - skillEfficiency) * 100, actor->AsActorValueOwner()->GetActorValue(ActorValue::kOneHanded));
            } else if (weapon->IsTwoHandedAxe() || weapon->IsTwoHandedSword()) {
                calculateSkillEfficiency(actor, ActorValue::kTwoHanded, &skillEfficiency);
                logger::info("Efficiency {}% (Two-Handed skill {})", (1 - skillEfficiency) * 100, actor->AsActorValueOwner()->GetActorValue(ActorValue::kTwoHanded));
            } else if (weapon->IsCrossbow() || weapon->IsBow()) {
                calculateSkillEfficiency(actor, ActorValue::kArchery, &skillEfficiency);
                logger::info("Efficiency {}% (Archery skill {})", (1 - skillEfficiency) * 100, actor->AsActorValueOwner()->GetActorValue(ActorValue::kArchery));
            } else {
                logger::info("You shouldn't be here!");
            }

            // costBase = ((weapon->GetWeight() * bashMult) + bashBase) * skillEfficiency;
            calculateStaminaCost(actualCost, bashBase, bashMult, skillEfficiency, &costBase);

            // Potential error
            BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModPowerAttackStamina, actor, weapon, &perkMult);
            logger::info("Perk mult: {}", perkMult);
        } else {
            logger::info("Bashes with {} ???", equip->GetName());
        }

        if (atkData->data.flags.any(AttackData::AttackFlag::kPowerAttack)) {
            logger::info("Is Power");

            logger::info("Stamina cost: {}", costBase * powerMult * perkMult * atkData->data.staminaMult);

            return costBase * powerMult * perkMult * atkData->data.staminaMult;
        }

        logger::info("Stamina cost: {}", costBase * atkData->data.staminaMult);

        return costBase * atkData->data.staminaMult;
    } else {
        float swingBase = mINI::GetIniFloat(mini, "Configurations", "swingBase");
        float swingMult = mINI::GetIniFloat(mini, "Configurations", "swingMult");

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
        logger::info("Swings with {} | Weight {}", weapon->GetName(), weapon->GetWeight());

        // float tester = 0;
        // Hooks::CombatStamina::calculateSkillEfficiency(actor, ActorValue::kOneHanded, &tester);
        // logger::info("Test value of One-Handed: {}", tester);

        float skillEfficiency = 0;
        if (weapon->IsOneHandedAxe() || weapon->IsOneHandedSword() || weapon->IsOneHandedMace() || weapon->IsOneHandedDagger()) {
            calculateSkillEfficiency(actor, ActorValue::kOneHanded, &skillEfficiency);
            logger::info("Efficiency {}% (One-Handed skill {})", (1 - skillEfficiency) * 100, actor->AsActorValueOwner()->GetActorValue(ActorValue::kOneHanded));
        } else if (weapon->IsTwoHandedAxe() || weapon->IsTwoHandedSword()) {
            calculateSkillEfficiency(actor, ActorValue::kTwoHanded, &skillEfficiency);
            logger::info("Efficiency {}% (Two-Handed skill {})", (1 - skillEfficiency) * 100, actor->AsActorValueOwner()->GetActorValue(ActorValue::kTwoHanded));
        } else {
            logger::info("You shouldn't be here!");
        }

        float actualCost = weapon->GetWeight() < 1 ? costBase : weapon->GetWeight();
        // costBase = ((actualCost * swingMult) + swingBase) * skillEfficiency;

        // At the minimum, 1 stamina must be spent
        calculateStaminaCost(actualCost, swingBase, swingMult, skillEfficiency, &costBase);

        BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModPowerAttackStamina, actor, weapon, &perkMult);
        logger::info("Perk mult: {}", perkMult);

        if (atkData->data.flags.any(AttackData::AttackFlag::kPowerAttack)) {
            logger::info("Is Power", actor->GetName());
            logger::info("Stamina cost: {}", costBase * powerMult * perkMult * atkData->data.staminaMult);

            return costBase * powerMult * perkMult * atkData->data.staminaMult;
        }

        logger::info("Stamina cost: {}", costBase * atkData->data.staminaMult);

        return costBase * atkData->data.staminaMult;
    }

    return _ActionStaminaCost(avOwner, atkData);
}

void Hooks::CombatStamina::calculateStaminaCost(float weight, float base, float mult, float efficiency, float* ret) {
    *ret = ((weight * mult) * efficiency) + base;
    if (*ret < 1) {
        *ret = 1;
    }
}

void Hooks::CombatStamina::calculateSkillEfficiency(Actor* actor, ActorValue av, float* ret) {
    float value = actor->AsActorValueOwner()->GetActorValue(av);
    float eff = 0;

    eff = value / 2;

    *ret = 1 - (floor(eff) / 100);
}

// Currently unused
void Hooks::CombatHit::HitImpact(Actor* target, HitData& hitData) {
    auto mini = mINI::GetIniFile("Data/SKSE/Plugins/DCS-DynamicCombatStamina.ini");
    bool enableContextualStagger = mINI::GetIniBool(mini, "Experimental", "enableContextualStagger");
    Actor* source = hitData.aggressor.get().get();

    if (!target || !source) {
        _HitImpact(target, hitData);
        return;
    }

    float& stgAmt = hitData.stagger;

    if (enableContextualStagger) {
        if (hitData.flags.any(RE::HitData::Flag::kBash) && !IsBlocking(target) && !IsAttacking(target) && !IsCasting(target)) {
            logger::info("Resist Stagger: Bashed while idle");
            hitData.stagger = 0.0F;
        }

        if (hitData.flags.any(RE::HitData::Flag::kPowerAttack) && IsBlocking(target)) {
            logger::info("Resist Stagger: Power attack is blocked");
            hitData.stagger = 0.0F;
        }
    }

    logger::info("{} stagger amount: {}", source->GetName(), stgAmt);

    _HitImpact(target, hitData);
}

bool Hooks::CombatAction::DoCombatAction(TESActionData* actData) {
    Actor* actor = actData->source.get()->As<Actor>();

    if (!actor) {
        return _DoCombatAction(actData);
    }

    float stamina = actor->AsActorValueOwner()->GetActorValue(ActorValue::kStamina);
    const char* event = actData->animEvent.c_str();

    if (stamina <= 0 && (strstr(event, "attack") || strstr(event, "bash") || strstr(event, "block"))) {
        logger::info("{} tries to attack, but have no stamina!", actor->GetName());

        return false;
    }

    return _DoCombatAction(actData);
}

void Hooks::CombatRegenerate::RegenerateCheck(Actor* actor, ActorValue av, float rate) {
    auto mini = mINI::GetIniFile("Data/SKSE/Plugins/DCS-DynamicCombatStamina.ini");
    bool enableRegenAttack = mINI::GetIniBool(mini, "Experimental", "enableRegenAttack");

    if (!actor) {
        _RestoreActorValue(actor, av, rate);
    }

    switch (av) {
        case ActorValue::kStamina:
            if (actor->IsAttacking() && !enableRegenAttack) {
                return;
            }
            break;
    }

    _RestoreActorValue(actor, av, rate);
}