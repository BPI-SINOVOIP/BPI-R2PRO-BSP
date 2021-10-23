/**
 * Copyright (C) STM 2013. All rights reserved.
 * This code is ST-Ericsson proprietary and confidential.
 * Any use of the code for whatever purpose is subject to
 * specific written permission of STM.
 *
 */

#define STR_TRACE_USER_TA "TEST_APP"

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <tee_api_defines.h>

#include "ta_testapp.h"

/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */

unsigned char str[] = "It's Test App!";
unsigned char output[] = "test_membuf_from_TA_to_CA";

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
				    TEE_Param  params[4], void **sess_ctx)
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
	IMSG("Hello Test App!\n");
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
	IMSG("Goodbye Test App!\n");
}

/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result TA_InvokeCommandEntryPoint(void *sess_ctx, uint32_t cmd_id,
				      uint32_t param_types, TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
						   TEE_PARAM_TYPE_MEMREF_INOUT,
						   TEE_PARAM_TYPE_MEMREF_INOUT,
						   TEE_PARAM_TYPE_NONE);

	(void)&sess_ctx;

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	switch (cmd_id) {
	case TA_TEST_APP_INC_VALUE:
		params[0].value.a++;
		params[0].value.b = params[0].value.a;
		return TEE_SUCCESS;

	case TA_TEST_APP_FILL_TEMP_BUF:
		params[1].memref.size = sizeof(str);
		TEE_MemMove(params[1].memref.buffer,
			    str, params[1].memref.size);
		return TEE_SUCCESS;

	case TA_TEST_APP_FILL_MEM_BUF:
		if (!TEE_MemCompare(params[2].memref.buffer,
				    "test_membuf_from_CA_to_TA",
				    params[2].memref.size)) {
			IMSG("membuf test : Pass!\n");
		} else {
			IMSG("membuf test : Fail! (mismatch string)\n");
			return TEE_ERROR_BAD_PARAMETERS;
		}
		TEE_MemMove(params[2].memref.buffer,
			    output, params[2].memref.size);
		return TEE_SUCCESS;

	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
