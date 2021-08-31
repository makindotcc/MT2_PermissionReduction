#pragma warning (disable : 4100)

#include "nmk.h"

PVOID callbackRegistrationHandle = NULL;

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath) {
	pDriverObject->DriverUnload = DriverUnload;

	DbgPrintEx(0, 0, "Welcome witam");

	UNICODE_STRING altitude;
	RtlInitUnicodeString(&altitude, L"320420");

	OB_OPERATION_REGISTRATION operationRegistrations[1];
	operationRegistrations[0].ObjectType = PsProcessType;
	operationRegistrations[0].Operations |= OB_OPERATION_HANDLE_CREATE;
	operationRegistrations[0].Operations |= OB_OPERATION_HANDLE_DUPLICATE;
	operationRegistrations[0].PreOperation = NmkObjectCallback;
	operationRegistrations[0].PostOperation = NULL;

	OB_CALLBACK_REGISTRATION callbackRegistration;
	callbackRegistration.Version = OB_FLT_REGISTRATION_VERSION;
	callbackRegistration.OperationRegistrationCount = 1;
	callbackRegistration.Altitude = altitude;
	callbackRegistration.OperationRegistration = operationRegistrations;
	callbackRegistration.RegistrationContext = NULL;

	NTSTATUS status = ObRegisterCallbacks(&callbackRegistration, &callbackRegistrationHandle);
	DbgPrintEx(0, 0, "Status: %X\n", status);

	return status;
}

NTSTATUS DriverUnload(PDRIVER_OBJECT pDriverObject) {
	if (callbackRegistrationHandle != NULL) {
		ObUnRegisterCallbacks(callbackRegistrationHandle);
	}
	return STATUS_SUCCESS;
}

OB_PREOP_CALLBACK_STATUS NmkObjectCallback(_In_ PVOID registrationContext, _Inout_ POB_PRE_OPERATION_INFORMATION preInfo) {
	if (preInfo->KernelHandle) {
		return OB_PREOP_SUCCESS;
	}
	if (preInfo->ObjectType != *PsProcessType) {
		return OB_PREOP_SUCCESS;
	}
	PEPROCESS oliwcia = preInfo->Object;
	UCHAR* currentImageName = PsGetProcessImageFileName(IoGetCurrentProcess());
	UCHAR* targetImageName = PsGetProcessImageFileName(oliwcia);

	switch (preInfo->Operation) {
	case OB_OPERATION_HANDLE_CREATE:
		if (strcmp((char*) currentImageName, "HackShield.exe") != 0) {
			return OB_PREOP_SUCCESS;
		}
		// pass netsh.exe, because client awaits for this process completion (?)
		if (strcmp((char*)targetImageName, "netsh.exe") == 0) {
			return OB_PREOP_SUCCESS;
		}
		preInfo->Parameters->CreateHandleInformation.DesiredAccess = 0;
		DbgPrintEx(0, 0, "Create: %s (%s): %X (org: %X)", currentImageName, targetImageName,
			preInfo->Parameters->CreateHandleInformation.DesiredAccess,
			preInfo->Parameters->CreateHandleInformation.OriginalDesiredAccess);
		break;
	case OB_OPERATION_HANDLE_DUPLICATE:
		break;
	default:
		break;
	}
	return OB_PREOP_SUCCESS;
}
