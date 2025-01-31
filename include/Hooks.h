/*
    BIG BIG THANKS to
    doodlum
    DTry
    For the hook call functions.
    List of the hook call and the repo where I found them can be found in the readme
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
            _actionStaminaCost = trampoline.write_call<5>(hook.address() + REL::Relocate(0x16E, 0x171), actionStaminaCost);
            logger::info("CombatStamina hook installed");
        }

    private:
        static float actionStaminaCost(RE::ActorValueOwner* avOwner, RE::BGSAttackData* atkData);
        static inline REL::Relocation<decltype(actionStaminaCost)> _actionStaminaCost;
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
            _hitImpact = trampoline.write_call<5>(hook.address() + REL::Relocate(0x3C0, 0x4A8), hitImpact);
            logger::info("CombatHit hook installed!");
        }

    private:
        static void hitImpact(RE::Actor* target, RE::HitData& hitData);
        static inline REL::Relocation<decltype(hitImpact)> _hitImpact;
    };

    class CombatAction {
    public:
        static void Install() {
            auto& trampoline = SKSE::GetTrampoline();
            REL::Relocation<std::uintptr_t> AttackActionBase{RELOCATION_ID(48139, 49170)};
            _DoCombatAction = trampoline.write_call<5>(AttackActionBase.address() + REL::Relocate(0x4D7, 0x435), DoCombatAction);
            logger::info("hook:OnAttackAction");
        }

    private:
        static bool DoCombatAction(RE::TESActionData* actData);

        static inline REL::Relocation<decltype(DoCombatAction)> _DoCombatAction;
    };

    typedef RE::TESObjectREFR*(_fastcall* _getEquippedShield)(RE::Actor* a_actor);
    inline static REL::Relocation<_getEquippedShield> getEquippedShield{RELOCATION_ID(37624, 38577)};

    static void Install() {
        SKSE::AllocTrampoline(1 << 8);
        CombatStamina::Install();
        CombatHit::Install();
        CombatAction::Install();
    };
}