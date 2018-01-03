#pragma once

#include "stdafx.h"

bool scheduleTask(bool install = true, char *location = NULL) {
	LPCWSTR wszTaskName = L"FixSurfaceKeyboard";
	HRESULT hr;

	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
		return 1;
	}

	hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0, NULL);
	if (FAILED(hr))
	{
		CoUninitialize();
		return 1;
	}

	ITaskService *pService = NULL;
	hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService);
	if (FAILED(hr))
	{
		CoUninitialize();
		return 1;
	}

	hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
	if (FAILED(hr))
	{
		pService->Release();
		CoUninitialize();
		return 1;
	}

	ITaskFolder *pRootFolder = NULL;
	hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
	if (FAILED(hr))
	{
		pService->Release();
		CoUninitialize();
		return 1;
	}

	pRootFolder->DeleteTask(_bstr_t(wszTaskName), 0);
	if (!install) {
		pRootFolder->Release();
		pService->Release();
		CoUninitialize();
		return 0;
	}

	ITaskDefinition *pTask = NULL;
	hr = pService->NewTask(0, &pTask);

	pService->Release();  // COM clean up.  Pointer is no longer used.
	if (FAILED(hr))
	{
		pRootFolder->Release();
		CoUninitialize();
		return 1;
	}

	//  ------------------------------------------------------
	//  Set to also run in battery
	
	ITaskSettings *pSettings = NULL;
	hr = pTask->get_Settings(&pSettings);
	if (FAILED(hr))
	{
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}
	hr = pSettings->put_DisallowStartIfOnBatteries(false);
	if (FAILED(hr))
	{
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}
	hr = pSettings->put_StopIfGoingOnBatteries(false);
	if (FAILED(hr))
	{
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}
	IIdleSettings *pIdleSettings = NULL;
	hr = pSettings->get_IdleSettings(&pIdleSettings);
	if (FAILED(hr))
	{
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}
	pIdleSettings->put_StopOnIdleEnd(false);
	hr = pSettings->put_IdleSettings(pIdleSettings);
	if (FAILED(hr))
	{
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}
	hr = pSettings->put_ExecutionTimeLimit(L"PT0S");
	pSettings->Release();
	if (FAILED(hr))
	{
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}

    //  ------------------------------------------------------
	//  Set to run elevated
	IPrincipal *pPrincipal = NULL;
	hr = pTask->get_Principal(&pPrincipal);
	if (FAILED(hr))
	{
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}

	hr = pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
	pPrincipal->Release();
	if (FAILED(hr))
	{
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}

	//  ------------------------------------------------------
	//  Get the trigger collection to insert the logon trigger.
	ITriggerCollection *pTriggerCollection = NULL;
	hr = pTask->get_Triggers(&pTriggerCollection);
	if (FAILED(hr))
	{
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}

	//  Add the logon trigger to the task.
	ITrigger *pTrigger = NULL;
	hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger);
	pTriggerCollection->Release();
	if (FAILED(hr))
	{
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}

	ILogonTrigger *pLogonTrigger = NULL;
	hr = pTrigger->QueryInterface(
		IID_ILogonTrigger, (void**)&pLogonTrigger);
	pTrigger->Release();
	if (FAILED(hr))
	{
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}

	hr = pLogonTrigger->put_Id(_bstr_t(L"Trigger1"));
	if (FAILED(hr))
	{
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}


	//  ------------------------------------------------------
	//  Add an Action to the task
	IActionCollection *pActionCollection = NULL;

	//  Get the task action collection pointer.
	hr = pTask->get_Actions(&pActionCollection);
	if (FAILED(hr))
	{
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}

	//  Create the action, specifying that it is an executable action.
	IAction *pAction = NULL;
	hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
	pActionCollection->Release();
	if (FAILED(hr))
	{
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}

	IExecAction *pExecAction = NULL;
	//  QI for the executable task pointer.
	hr = pAction->QueryInterface(
		IID_IExecAction, (void**)&pExecAction);
	pAction->Release();
	if (FAILED(hr))
	{
		printf("\nQueryInterface call failed for IExecAction: %x", hr);
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}

	//  Set the path of the executable
	hr = pExecAction->put_Path(_bstr_t(location));
	pExecAction->Release();
	if (FAILED(hr))
	{
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}

	//  ------------------------------------------------------
	//  Save the task in the root folder.
	IRegisteredTask *pRegisteredTask = NULL;

	hr = pRootFolder->RegisterTaskDefinition(
		_bstr_t(wszTaskName),
		pTask,
		TASK_CREATE_OR_UPDATE,
		_variant_t(L"S-1-5-32-544"), // Builtin\\Administrators
		_variant_t(),
		TASK_LOGON_GROUP,
		_variant_t(L""),
		&pRegisteredTask);
	if (FAILED(hr))
	{
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}

	IRunningTask *pRunningTask = NULL;

	pRegisteredTask->Run(_variant_t(L""), &pRunningTask);

	// Clean up
	pRootFolder->Release();
	pTask->Release();
	pRegisteredTask->Release();
	CoUninitialize();
	return 0;
}