/*
 * Copyright (c) 2014, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <tee_client_api.h>
#include <ta_testapp.h>

#define BUF_SIZE 128

unsigned char membuf_input[] = "test_membuf_from_CA_to_TA";

int main(void)
{
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = TA_TEST_APP_UUID;
	uint32_t err_origin = 0;

	TEEC_SharedMemory shm = {
		.size = BUF_SIZE,
		.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT,
	};

	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS) {
		printf("TEEC_InitializeContext failed with code 0x%x\n", res);
		goto exit;
	}

	/*
	* Open a session to the "test app" TA, the TA will print "Hello
	* Test App!" in the log when the session is created.
	*/
	res = TEEC_OpenSession(&ctx, &sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS) {
		printf("TEEC_Opensession failed with code 0x%x origin 0x%x\n",
		       res, err_origin);
		goto exit;
	}

	/*
	* Execute a function in the TA by invoking it, in this case
	* we're incrementing a number.
	*
	* The value of command ID part and how the parameters are
	* interpreted is part of the interface provided by the TA.
	*/

	/*
	* Prepare the argument. The first parameter is about value,
	* the second parameter is about temp mem, the third parameter
	* is about shared mem, the last parameter is unused.
	*/
	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT,
					 TEEC_MEMREF_TEMP_INOUT,
					 TEEC_MEMREF_PARTIAL_INOUT,
					 TEEC_NONE);

	op.params[0].value.a = 42;
	op.params[0].value.b = 0x88888888;
	op.params[1].tmpref.size = BUF_SIZE;
	op.params[1].tmpref.buffer = (void *)malloc(op.params[1].tmpref.size);
	memset(op.params[1].tmpref.buffer, 0, BUF_SIZE);

	op.params[2].memref.parent = &shm;
	op.params[2].memref.offset = 0;
	op.params[2].memref.size = sizeof(membuf_input);

	res = TEEC_AllocateSharedMemory(&ctx, &shm);
	if (res != TEEC_SUCCESS) {
		printf("TEEC_AllocateSharedMemory failed ");
		printf("with code 0x%x origin 0x%x\n",
		       res, err_origin);
		goto exit;
	}

	shm.size = sizeof(membuf_input);
	memcpy(shm.buffer, membuf_input, shm.size);

	res = TEEC_InvokeCommand(&sess, TA_TEST_APP_INC_VALUE,
				 &op, &err_origin);
	if (res != TEEC_SUCCESS) {
		printf("TEEC_InvokeCommand failed with code 0x%x origin 0x%x\n",
		       res, err_origin);
		goto exit;
	}

	if (op.params[0].value.a == 43 &&
	    op.params[0].value.b == op.params[0].value.a)
		printf("test value : Pass!\n");
	else
		printf("test value : Fail! (mismatch values)\n");

	res = TEEC_InvokeCommand(&sess, TA_TEST_APP_FILL_TEMP_BUF,
				 &op, &err_origin);
	if (res != TEEC_SUCCESS) {
		printf("TEEC_InvokeCommand failed with code 0x%x origin 0x%x\n",
		       res, err_origin);
		goto exit;
	}

	if (op.params[1].tmpref.size != BUF_SIZE &&
	    !strcmp(op.params[1].tmpref.buffer, "It's Test App!"))
		printf("test tembuf : Pass!\n");
	else
		printf("test tembuf : Fail! (mismatch string)\n");

	res = TEEC_InvokeCommand(&sess, TA_TEST_APP_FILL_MEM_BUF,
				 &op, &err_origin);
	if (res != TEEC_SUCCESS) {
		printf("TEEC_InvokeCommand failed with code 0x%x origin 0x%x\n",
		       res, err_origin);
		goto exit;
	}

	if (shm.size == sizeof(membuf_input) &&
	    !strcmp(shm.buffer, "test_membuf_from_TA_to_CA"))
		printf("test shmbuf : Pass!\n");
	else
		printf("test shmbuf : Fail! (mismatch string)\n");

exit:
	/*
	* We're done with the TA, close the session and
	* destroy the context.
	*
	* The TA will print "Goodbye Test App!" in the
	* log when the session is closed.
	*/

	TEEC_ReleaseSharedMemory(&shm);
	TEEC_CloseSession(&sess);
	TEEC_FinalizeContext(&ctx);

	return res;
}

