/*==============================================================================
*
*            TEXAS INSTRUMENTS INCORPORATED PROPRIETARY INFORMATION
*
*   Property of Texas Instruments
*   For Use by Texas Instruments and/or its Licensees Only.
*   Restricted rights to use, duplicate or disclose this code are
*   granted through contract.
*   Unauthorized reproduction and/or distribution are strictly prohibited.
*   This product is protected under copyright law and trade secret law as an
*   Unpublished work.
*   Created 2011, (C) Copyright 2011 Texas Instruments.  All rights reserved.
*
*   Component  :
*
*   Filename   : 	ti/xdais/ialg.h
*
*   Description:	This header defines all types, constants, and functions
*              		defined by xDAIS for algorithms.
*
*              		This is the xDAIS IALG interface.
*
*=============================================================================*/

#ifndef ti_xdais_IALG_
#define ti_xdais_IALG_

#ifdef __cplusplus
extern "C" {
#endif

/** @ingroup    ti_xdais_IALG */
/*@{*/

/*---------------------------*/
/*    TYPES AND CONSTANTS    */
/*---------------------------*/

#define IALG_DEFMEMRECS 4   /**< Default number of memory records. */
#define IALG_OBJMEMREC  0   /**< Memory record index of instance object. */
#define IALG_SYSCMD     256 /**< Minimum "system" IALG_Cmd value. */

#define IALG_EOK        0   /**< Successful return status code. */
#define IALG_EFAIL      -1  /**< Unspecified error return status code. */

/**
 *  @brief      Memory attributes.
 */
typedef enum IALG_MemAttrs {
    IALG_SCRATCH,           /**< Scratch memory. */
    IALG_PERSIST,           /**< Persistent memory. */
    IALG_WRITEONCE          /**< Write-once persistent memory. */
} IALG_MemAttrs;

#define IALG_MPROG  0x0008  /**< Program memory space bit. */
#define IALG_MXTRN  0x0010  /**< External memory space bit. */

/**
 *  @brief      Defined memory spaces.
 */
/*
 *  ======== IALG_MemSpace ========
 */
typedef enum IALG_MemSpace {
    IALG_EPROG =            /**< External program memory */
        IALG_MPROG | IALG_MXTRN,

    IALG_IPROG =            /**< Internal program memory */
        IALG_MPROG,

    IALG_ESDATA =           /**< Off-chip data memory (accessed sequentially) */
        IALG_MXTRN + 0,

    IALG_EXTERNAL =         /**< Off-chip data memory (accessed randomly) */
        IALG_MXTRN + 1,

    IALG_DARAM0 = 0,        /**< Dual access on-chip data memory */
    IALG_DARAM1 = 1,        /**< Block 1, if independant blocks required */

    IALG_SARAM  = 2,        /**< Single access on-chip data memory */
    IALG_SARAM0 = 2,        /**< Block 0, equivalent to IALG_SARAM */
    IALG_SARAM1 = 3,        /**< Block 1, if independant blocks required */

    IALG_DARAM2 = 4,        /**< Block 2, if a 3rd independent block required */
    IALG_SARAM2 = 5         /**< Block 2, if a 3rd independent block required */
} IALG_MemSpace;

/*
 *  ======== IALG_isProg ========
 */
#define IALG_isProg(s) (        \
    (((int)(s)) & IALG_MPROG)   \
)

/*
 *  ======== IALG_isOffChip ========
 */
#define IALG_isOffChip(s) (     \
    (((int)(s)) & IALG_MXTRN)   \
)


/**
 *  @brief      Memory records.
 */
typedef struct IALG_MemRec {
    Uns             size;       /**< Size in MAU of allocation */
    Int             alignment;  /**< Alignment requirement (MAU) */
    IALG_MemSpace   space;      /**< Allocation space */
    IALG_MemAttrs   attrs;      /**< Memory attributes */
    Void            *base;      /**< Base address of allocated buf */
} IALG_MemRec;


/**
 *  @brief      Algorithm instance object definition.
 *
 *  @remarks    All xDAIS algorithm instance objects <b>must</b> have this
 *              structure as their first element.  However, they do not
 *              need to initialize it; initialization of this sub-structure
 *              is done by the "framework".
 */
typedef struct IALG_Obj {
    struct IALG_Fxns *fxns;     /**< Pointer to IALG function table. */
} IALG_Obj;


/**
 *  @brief      Handle to an algorithm instance object.
 */
typedef struct IALG_Obj *IALG_Handle;


/**
 *  @brief      Algorithm instance creation parameters.
 *
 *  @remarks    All XDAS algorithm parameter structures <b>must</b> have a this
 *              as their first element.
 */
typedef struct IALG_Params {
    Int size;       /**< Number of MAU in the structure */
} IALG_Params;


/**
 *  @brief      Pointer to algorithm specific status structure.
 *
 *  @remarks    All xDAIS algorithm parameter structures <b>must</b> have this
 *              as their first element.
 */
typedef struct IALG_Status {
    Int size;       /**< Number of MAU in the structure */
} IALG_Status;


/**
 *  @brief      Algorithm specific command.
 *
 *  @remarks    This command is used in conjunction with IALG_Status to get
 *              and set algorithm specific attributes via the algControl()
 *               method.
 */
typedef unsigned int IALG_Cmd;


/**
 *  @brief      Defines the fields and methods that must be supplied by all
 *              xDAIS algorithms.
 */
/*
 *      algAlloc()        - apps call this to query the algorithm about
 *                          its memory requirements. Must be non-NULL.
 *      algControl()      - algorithm specific control operations.  May be
 *                          NULL; NULL => no operations supported.
 *      algDeactivate()   - notification that current instance is about to
 *                          be "deactivated".  May be NULL; NULL => do nothing.
 *      algFree()         - query algorithm for memory to free when removing
 *                          an instance.  Must be non-NULL.
 *      algInit()         - apps call this to allow the algorithm to
 *                          initialize memory requested via algAlloc().  Must
 *                          be non-NULL.
 *      algMoved()        - apps call this whenever an algorithms object or
 *                          any pointer parameters are moved in real-time.
 *                          May be NULL; NULL => object can not be moved.
 *      algNumAlloc()     - query algorithm for number of memory requests.
 *                          May be NULL; NULL => number of mem recs is less
 *                          then IALG_DEFMEMRECS.
 */
typedef struct IALG_Fxns {
/**
 *  @brief      Unique pointer that identifies the module
 *              implementing this interface.
 */
    Void    *implementationId;

/**
 *  @brief      Notification to the algorithm that its memory
 *              is "active" and algorithm processing methods
 *              may be called.
 *
 *  @param[in]  handle          Handle to an algorithm instance.
 *
 *  @remarks    algActivate() initializes any of the instance's scratch
 *              buffers using the persistent memory that is part of the
 *              algorithm's instance object.
 *
 *  @remarks    The implementation of algActivate() is optional.  The
 *              algActivate() method should only be implemented if a module
 *              wants to factor out initialization code that can be executed
 *              once prior to processing multiple consecutive frames of data.
 *
 *  @remarks    If a module does not implement this method, the algActivate()
 *              field in the module's static function table (of type
 *              IALG_Fxns) must be set to @c NULL.  This is equivalent to
 *              the following implementation:
 *  @code
 *      Void algActivate(IALG_Handle handle)
 *      {
 *      }
 *  @endcode
 *
 *  @pre        algActivate() can only be called after a successful return
 *              from algInit().
 *
 *  @pre        @c handle must be a valid handle for the algorithm's
 *              instance object.
 *
 *  @pre        No other algorithm method is currently being run on this
 *              instance. This method never preempts any other method on
 *              the same instance.
 *
 *  @pre        If the algorithm has implemented the IDMA2 interface,
 *              algActivate() can only be called after a successful return
 *              from dmaInit().
 *
 *  @post       All methods related to the algorithm may now be executed
 *              by client (subject to algorithm specific restrictions).
 *
 *  @sa         algDeactivate().
 */
    Void    (*algActivate)(IALG_Handle handle);

/**
 *  @brief      Apps call this to query the algorithm about
 *              its memory requirements.  Must be non-NULL.
 *
 *  @param[in]  params          Algorithm specific attributes.
 *  @param[out] parentFxns      Parent algorithm functions.
 *  @param[out] memTab          array of memory records.
 *
 *  @remarks    algAlloc() returns a table of memory records that
 *              describe the size, alignment, type and memory space of
 *              all buffers required by an algorithm (including the
 *              algorithm's instance object itself).  If successful,
 *              this function returns a positive non-zero value
 *              indicating the number of records initialized.  This
 *              function can never initialize more memory records than
 *              the number returned by algNumAlloc().
 *
 *  @remarks    If algNumAlloc() is not implemented, the maximum number
 *              of initialized memory records is #IALG_DEFMEMRECS.
 *
 *  @remarks    The first argument to algAlloc() is a pointer to the creation
 *              parameters for the instance of the algorithm object to be
 *              created.  This pointer is algorithm-specific; i.e., it points
 *              to a structure that is defined by each particular algorithm.
 *              This pointer may be @c NULL; however, in this case, algAlloc()
 *              must assume default creation parameters and must not fail.
 *
 *  @remarks    The second argument to algAlloc() is an optional parameter.
 *              algAlloc() may return a pointer to its parent's IALG functions.
 *              If this output value is assigned a non-NULL value, the client
 *              must create the parent instance object using the designated
 *              IALG functions pointer.  The parent instance object must then
 *              be passed to algInit().
 *
 *  @remarks    algAlloc() may be called at any time and it must be idempotent;
 *              i.e., it can be called repeatedly without any side effects,
 *              and always returns the same result.
 *
 *  @pre        The number of memory records in the array @c memTab[] is no
 *              less than the number returned by algNumAlloc().
 *
 *  @pre        @c *parentFxns is a valid pointer to an IALG_Fxns pointer
 *              variable.
 *
 *  @post       If the algorithm needs a parent object to be created, the
 *              pointer @c *parentFxns is set to a non-NULL value that points
 *              to a valid IALG_Fxns structure, the parent's IALG
 *              implementation.  Otherwise, this pointer is not set.  algAlloc()
 *              may elect to ignore the @c parentFxns pointer altogether.
 *
 *  @post       For each memory descriptor in memTab with an IALG_WRITEONCE
 *              attribute, the algorithm has either set the base field to a
 *              non-NULL value, which is the address of a statically
 *              allocated and initialized memory buffer of the indicated
 *              'size', or has set the base field to @c NULL, thereby
 *              requiring the memory for the buffer to be provided by the
 *              client.
 *
 *  @post       Exactly @n elements of the @c memTab[] array are initialized,
 *              where @c n is the return value from this operation.
 *
 *  @post       For each memory descriptor in @c memTab with an IALG_PERSIST or
 *              IALG_SCRATCH attribute, the algorithm does not set its base
 *              field.
 *
 *  @post       @c memTab[0] defines the memory required for the instance's
 *              object and this object's first field is an IALG_Obj structure.
 *
 *  @post       @c memTab[0] is requested as persistent memory.
 *
 *  @sa         algFree()
 */
    Int     (*algAlloc)(const IALG_Params *params,
        struct IALG_Fxns **parentFxns, IALG_MemRec *memTab);

/**
 *  @brief      Algorithm specific control and status.
 *
 *  @param[in]  handle          Algorithm instance handle.
 *  @param[in]  cmd             Algorithm specific command.
 *  @param[out] status          Algorithm specific status.
 *
 *  @remarks    algControl() sends an algorithm specific command, @c cmd, and
 *              an input/output status buffer pointer to an algorithm's
 *              instance object.
 *
 *  @remarks    In preemptive execution environments, algControl() may preempt
 *              a module's other metods (for example, its processing methods).
 *
 *  @remarks    The implementation of algControl() is optional.  If a module
 *              does not implement this method, the algControl() field in
 *              the module's static function table (of type IALG_Fxns) must
 *              be set to @c NULL.  This is equivalent to the following
 *              implementation:
 *  @code
 *      Void algControl(IALG_Handle handle, IALG_Cmd cmd, IALG_Status *status)
 *      {
 *          return (IALG_EFAIL);
 *      }
 *  @endcode
 *
 *  @pre        algControl() can only be called after a successful return
 *              from algInit().
 *
 *  @pre        @c handle must be a valid handle for the algorithm's instance
 *              object.
 *
 *  @pre        Algorithm specific @c cmd values are always less than
 *              IALG_SYSCMD.
 *
 *  @post       If the @c cmd value is not recognized by the algorithm, the
 *              return value is not equal to IALG_EOK.
 *
 *  @sa         algInit()
 */
    Int     (*algControl)(IALG_Handle handle, IALG_Cmd cmd,
        IALG_Status *status);

/**
 *  @brief      Save all persistent data to non-scratch memory.
 *
 *  @param[in]  handle          Algorithm instance handle.
 *
 *  @remarks    algDeactivate() saves any persistent information to non-scratch
 *              buffers using the persistent memory that is part of the
 *              algorithm's instance object.
 *
 *  @remarks    @c handle is used by the algorithm to identify the various
 *              buffers that must be saved prior to the next cycle of
 *              algActivate() and processing.
 *
 *  @remarks    The implementation of algDeactivate() is optional.  The
 *              algDeactivate() method is only implemented if a module wants
 *              to factor out initialization code that can be executed once
 *              prior to processing multiple consecutive frames of data.
 *
 *  @remarks    If a module does not implement this method, the
 *              algDeactivate() field in the module's static function table
 *              (of type IALG_Fxns) must be set to @c NULL.  This is
 *              equivalent to the following implementation:
 *  @code
 *      Void algDeactivate(IALG_Handle handle)
 *      {
 *      }
 *  @endcode
 *
 *  @pre        algDeactivate() can only be called after a successful return
 *              from algInit().
 *
 *  @pre        The instance object is currently "active"; i.e., all instance
 *              memory is active and if an algActivate() method is defined,
 *              it has been called.
 *
 *  @pre        @c handle must be a valid handle for the algorithm's instance
 *              object.
 *
 *  @pre        No other algorithm method is currently being run on this
 *              instance.  This method never preempts any other method on the
 *              same instance.
 *
 *  @post       No methods related to the algorithm may now be executed by
 *              the client; only algActivate() or algFree() may be called.
 *
 *  @post       All instance scratch memory may be safely overwritten.
 *
 *  @sa         algActivate()
 */
    Void    (*algDeactivate)(IALG_Handle handle);

/**
 *  @brief      Apps call this to allow the algorithm to initialize memory
 *              requested via algAlloc().  Must be non-NULL.
 *
 *  @param[in]  handle          Algorithm instance handle.
 *  @param[out] memTab          Output array of memory records.
 *
 *  @remarks    algFree() returns a table of memory records that describe
 *              the base address, size, alignment, type and memory space
 *              of all buffers previously allocated for the algorithm's
 *              instance (including the algorithm's instance object itself)
 *              specified by @c handle.  This function always returns a
 *              positive non-zero value indicating the number of records
 *              initialized.  This function can never initialize more memory
 *              records than the value returned by algNumAlloc().
 *
 *  @pre        The @c memTab[] array contains at least algNumAlloc() records.
 *
 *  @pre        @c handle must be a valid handle for the algorithm's instance
 *              object.
 *
 *  @pre        If the prior call to algAlloc() returned a non-NULL parent
 *              functions pointer, then the parent instance must be an active
 *              instance object created via that function pointer.
 *
 *  @pre        No other agorithm method is currently being run on this
 *              instance.  This method never preempts any other method on the
 *              same instance.
 *
 *  @post       @c memTab[] contains pointers to all of the memory passed to
 *              the algorithm via algInit().
 *
 *  @post       The size and alignment fields contain the same values passed
 *              to the client via algAlloc(); i.e., if the client makes changes
 *              to the values returned via algAlloc() and passes these new
 *              values to algInit(), the algorithm is not responsible for
 *              retaining any such changes.
 *
 *  @sa         algAlloc()
 */
    Int     (*algFree)(IALG_Handle handle, IALG_MemRec *memTab);

/**
 *  @brief      Initialize an algorithm's instance object.  Must be non-NULL.
 *
 *  @param[in]  handle          Algorithm instance handle.  This is a pointer
 *                              to an initialized IALG_Obj structure.  Its
 *                              value is identical to the memTab[0].base.
 *  @param[in]  memTab          Array of allocated buffers.
 *  @param[in]  parent          Handle of algorithm's parent instance.
 *  @param[in]  params          Pointer to algorithm's instance parameters.
 *
 *  @remarks    algInit() performs all initialization necessary to complete the
 *              run-time creation of an algorithm's instance object.  After a
 *              successful return from algInit(), the algorithm's instance
 *              object is ready to be used to process data.
 *
 */
    Int     (*algInit)(IALG_Handle handle, const IALG_MemRec *memTab,
        IALG_Handle parent, const IALG_Params *params);

/**
 *  @brief      Notify algorithm instance that instance memory has been
 *              relocated.
 *
 *  @param[in]  handle          Algorithm instance handle.
 *  @param[in]  memTab          Array of allocated buffers.
 *  @param[in]  parent          Handle of algorithm's parent instance.
 *  @param[in]  params          Pointer to algorithm's instance parameters.
 *
 *  @remarks    algMoved() performs any reinitialization necessary to insure
 *              that, if an algorithm's instance object has been moved by the
 *              client, all internal data references are recomputed.
 *
 *  @remarks    The implementation of algMoved() is optional.  However, it is
 *              highly recommended that this method be implemented.  If a
 *              module does not implement this method, the algMoved() field
 *              in the module's static function table (of type IALG_Fxns) must
 *              be set to @c NULL.  This is equivalent to asserting that the
 *              algorithm's instance objects cannot be moved.
 */
    Void    (*algMoved)(IALG_Handle handle, const IALG_MemRec *memTab,
        IALG_Handle parent, const IALG_Params *params);

/**
 *  @brief      Number of memory allocation requests required.
 *
 *  @remarks    algNumAlloc() returns the maximum number of memory allocation
 *              requests that the algAlloc() method requires.  This operation
 *              allows clients to allocate sufficient space to call the
 *              algAlloc() method or fail because insufficient space exists
 *              to support the creation of the algorithm's instance object.
 *              algNumAlloc() may be called at any time, and it must be
 *              idempotent; i.e., it can be called repeatedly without any
 *              side effects, and always returns the same result.
 *
 *  @remarks    algNumAlloc() is optional; if it is not implemented, the
 *              maximum number of memory records for algAlloc() is assumed
 *              to be #IALG_DEFMEMRECS.  This is equivalent to the following
 *              implementation:
 *  @code
 *      Void algNumAlloc(Void)
 *      {
 *          return (IALG_DEFNUMRECS);
 *      }
 *  @endcode
 *
 *  @remarks    If a module does not implement this method, the algNumAlloc()
 *              field in the module's static function table (of type IALG_Fxns)
 *              must be set to @c NULL.
 */
    Int     (*algNumAlloc)(Void);
} IALG_Fxns;

/*@}*/

#ifdef __cplusplus
}
#endif

#endif  /* ti_xdais_IALG_ */
/*
 *  @(#) ti.xdais; 1, 2.0, 0,100; 3-31-2007 20:39:11; /db/wtree/library/trees/dais-h11x/src/
 */

