/**
 * Copyright (C) STM 2013. All rights reserved.
 * This code is ST-Ericsson proprietary and confidential.
 * Any use of the code for whatever purpose is subject to
 * specific written permission of STM.
 *
 */

#define STR_TRACE_USER_TA "TEST_APP_STORAGE"

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <tee_api_defines.h>

#include "ta_testapp_storage.h"

/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */

TEE_Result TA_CreateEntryPoint(void)
{
	return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
				    TEE_Param params[4], void **sess_ctx)
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Unused parameters */
	(void)&params;
	(void)&sess_ctx;

	/*
	 * The IMSG() macro is non-standard, TEE Internal API doesn't
	 * specify any means to logging from a TA.
	 */
	IMSG("Hello Test Storage!\n");
	/* If return value != TEE_SUCCESS the session will not be created. */
	return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void *sess_ctx)
{
	(void)&sess_ctx; /* Unused parameter */
	IMSG("Goodbye Test Storage!\n");
}

/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result TA_InvokeCommandEntryPoint(void *sess_ctx, uint32_t cmd_id,
				      uint32_t param_types, TEE_Param params[4])
{
	TEE_Result result;
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	(void)&sess_ctx;
	(void)&params;

	switch (cmd_id) {
	case TA_TEST_APP_STORAGE:
	{
		TEE_ObjectHandle *obj = TEE_Malloc(sizeof(TEE_ObjectHandle), 0);

		uint32_t storageID = TEE_STORAGE_PRIVATE;
		uint32_t flags = TEE_DATA_FLAG_ACCESS_READ |
				TEE_DATA_FLAG_SHARE_READ |
				TEE_DATA_FLAG_ACCESS_WRITE |
				TEE_DATA_FLAG_SHARE_WRITE |
				TEE_DATA_FLAG_ACCESS_WRITE_META |
				TEE_DATA_FLAG_OVERWRITE;
		uint32_t objectID = 1;

		char filename[64] = "TA_Storage";
		char wbuf[64] = "TA Secure Storage Test";
		char rbuf[64];
		uint32_t count;

		result = TEE_CreatePersistentObject(storageID, &objectID,
						    sizeof(uint32_t), flags,
						    NULL, filename,
						    sizeof(filename), obj);
		if (result != TEE_SUCCESS) {
			IMSG("failed to creat storage with res = %x\n", result);
			return result;
		}
		IMSG("TEE_CreatePersistentObject success !\n");

		result = TEE_WriteObjectData(*obj, wbuf, sizeof(wbuf));
		if (result != TEE_SUCCESS) {
			IMSG("failed to write storage with res = %x\n", result);
			return result;
		}
		IMSG("TEE_WriteObjectData success !\n");

		result = TEE_SeekObjectData(*obj, 0, TEE_DATA_SEEK_SET);
		if (result != TEE_SUCCESS) {
			IMSG("failed to seek storage with res = %x\n", result);
			return result;
		}
		IMSG("TEE_SeekObjectData success !\n");

		result = TEE_ReadObjectData(*obj, rbuf, sizeof(rbuf), &count);
		if (result != TEE_SUCCESS) {
			IMSG("failed to read storage with res = %x\n", result);
			return result;
		}
		IMSG("TEE_ReadObjectData success !\n");

		if (!TEE_MemCompare(wbuf, rbuf, sizeof(rbuf)))
			IMSG("TA Storage : verify success\n");
		else
			IMSG("TA Storage : verify failed\n");

		IMSG("before TEE_CloseAndDeletePersistentObject");
		TEE_CloseAndDeletePersistentObject(*obj);
		IMSG("after TEE_CloseAndDeletePersistentObject");

		return TEE_SUCCESS;
	}
	default:
		IMSG("unknown command: 0x%x\n", cmd_id);
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
