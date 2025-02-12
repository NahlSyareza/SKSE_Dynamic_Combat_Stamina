/*
    BIG BIG THANKS to
    doodlum
    DTry
    For the hook call functions.
    List of the hook call and the repo where I found them can be found in the readme
    THANKS TO COLINSWRATH FOR PerkEntry APPLYING METHOD

    https://github.com/colinswrath/BladeAndBlunt/blob/main/include/patches/BashBlockStaminaPatch.h (line 82)
*/

namespace Hooks {
    class CombatStamina {
    public:
        static CombatStamina* GetSingleton() {
            static CombatStamina singleton;
            return &singleton;
        }

        static void Install() {
            auto& trampoline = SKSE::GetTrampoline();
            REL::Relocation<uintptr_t> hook{RELOCATION_ID(37650, 38603)};  // SE:627930 + 16E => 3BEC90 AE:64D350 + 171 => 3D6720
            _ActionStaminaCost = trampoline.write_call<5>(hook.address() + REL::Relocate(0x16E, 0x171), ActionStaminaCost);
            logger::info("CombatStamina hook installed (credits to doodlum, DTry, colinswrath, and everyone from mrowpurr discord server)");
        }

    private:
        static float ActionStaminaCost(RE::ActorValueOwner* avOwner, RE::BGSAttackData* atkData);
        static inline REL::Relocation<decltype(ActionStaminaCost)> _ActionStaminaCost;
    };

    class CombatHit {
    public:
        static CombatHit* GetSingleton() {
            static CombatHit singleton;
            return &singleton;
        }

        bool appliedStagger = false;
        std::thread::id staggerThread;

        static void TryStagger(RE::Actor* a_target, float a_staggerMult, RE::Actor* a_aggressor) {
            GetSingleton()->appliedStagger = true;
            GetSingleton()->staggerThread = std::this_thread::get_id();
            using func_t = decltype(&TryStagger);
            REL::Relocation<func_t> func{REL::RelocationID(36700, 37710)};
            func(a_target, a_staggerMult, a_aggressor);
        }

        static void Install() {
            auto& trampoline = SKSE::GetTrampoline();
            REL::Relocation<uintptr_t> hook{REL::RelocationID(37673, 38627)};
            _HitImpact = trampoline.write_call<5>(hook.address() + REL::Relocate(0x3C0, 0x4A8), HitImpact);
            logger::info("CombatHit hook installed (credits to doodlum, DTry, colinswrath, and everyone from mrowpurr discord server)");
        }

    private:
        static void HitImpact(RE::Actor* target, RE::HitData& hitData);
        static inline REL::Relocation<decltype(HitImpact)> _HitImpact;
    };

    class CombatAction {
    public:
        static void Install() {
            auto& trampoline = SKSE::GetTrampoline();
            REL::Relocation<std::uintptr_t> AttackActionBase{RELOCATION_ID(48139, 49170)};
            _DoCombatAction = trampoline.write_call<5>(AttackActionBase.address() + REL::Relocate(0x4D7, 0x435), DoCombatAction);
            logger::info("NPC CombatAction hook installed (credits to doodlum, DTry, colinswrath, and everyone from mrowpurr discord server)");
        }

    private:
        static bool DoCombatAction(RE::TESActionData* actData);
        static inline REL::Relocation<decltype(DoCombatAction)> _DoCombatAction;
    };

    typedef RE::TESObjectREFR*(_fastcall* _getEquippedShield)(RE::Actor* a_actor);
    inline static REL::Relocation<_getEquippedShield> getEquippedShield{RELOCATION_ID(37624, 38577)};

    static void Install() {
        SKSE::AllocTrampoline(1 << 8);
        Hooks::CombatStamina::Install();
        Hooks::CombatHit::Install();
        Hooks::CombatAction::Install();
    }
}