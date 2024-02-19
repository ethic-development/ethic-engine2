#include "anti_module.hpp"

void anti_module::initiate()
{
    this->setup_process_mitigations();
}

void anti_module::setup_process_mitigations()
{
    PROCESS_MITIGATION_DYNAMIC_CODE_POLICY pm_dynamic_code_policy{};
    pm_dynamic_code_policy.AllowRemoteDowngrade = 0;
    pm_dynamic_code_policy.ProhibitDynamicCode = 1;
    SetProcessMitigationPolicy(ProcessDynamicCodePolicy,
        &pm_dynamic_code_policy, sizeof(pm_dynamic_code_policy));

    PROCESS_MITIGATION_DEP_POLICY pm_dep_policy{};
    pm_dep_policy.Enable = 1;
    pm_dep_policy.Permanent = 1;
    SetProcessMitigationPolicy(ProcessDEPPolicy,
        &pm_dep_policy, sizeof(pm_dep_policy));

    PROCESS_MITIGATION_CONTROL_FLOW_GUARD_POLICY pm_control_flow_guard_policy{};
    pm_control_flow_guard_policy.EnableControlFlowGuard = 1;
    pm_control_flow_guard_policy.StrictMode = 1;
    SetProcessMitigationPolicy(ProcessControlFlowGuardPolicy,
        &pm_control_flow_guard_policy, sizeof(pm_control_flow_guard_policy));

    PROCESS_MITIGATION_BINARY_SIGNATURE_POLICY pm_binary_signature_policy{};
    pm_binary_signature_policy.MicrosoftSignedOnly = 1;
    SetProcessMitigationPolicy(ProcessSignaturePolicy,
        &pm_binary_signature_policy, sizeof(pm_binary_signature_policy));

    printf("Module prevention is active.\n");
}