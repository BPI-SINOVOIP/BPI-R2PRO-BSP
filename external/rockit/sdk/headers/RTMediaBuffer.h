/*
 * Copyright 2019 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * author: martin.cheng@rock-chips.com
 *   date: 2019/01/04
 * module: RTMediaBuffer
 * author: martin.cheng@rock-chips.com
 *   date: 2019/11/01
 * module: add RTBufferListener.
 */

#ifndef SRC_RT_MEDIA_INCLUDE_RTMEDIABUFFER_H_
#define SRC_RT_MEDIA_INCLUDE_RTMEDIABUFFER_H_

#include <map>

#include "RTObject.h"               // NOLINT
#include "rt_metadata.h"            // NOLINT
#include "RTMediaData.h"            // NOLINT
#include "rt_mutex.h"               // NOLINT

enum RtMediaBufferStatus {
    RT_MEDIA_BUFFER_STATUS_UNKNOWN = 0,
    RT_MEDIA_BUFFER_STATUS_UNUSED,
    RT_MEDIA_BUFFER_STATUS_READY,
    RT_MEDIA_BUFFER_STATUS_USING,
    RT_MEDIA_BUFFER_STATUS_INFO_CHANGE,
    RT_MEDIA_BUFFER_STATUS_PAST,
    RT_MEDIA_BUFFER_STATUS_UNSUPPORT,
    RT_MEDIA_BUFFER_STATUS_REALLOC,
    RT_MEDIA_BUFFER_STATUS_BOTTOM,
};

enum RTMediaBufferSite {
    RT_MEDIA_BUFFER_SITE_BY_US = 0,
    RT_MEDIA_BUFFER_SITE_DECODER,
    RT_MEDIA_BUFFER_SITE_RENDER,
    RT_MEDIA_BUFFER_SITE_ABANDON,
    RT_MEDIA_BUFFER_SITE_BOTTOM,
};

class RTBufferListener {
 public:
    RTBufferListener() {}
    virtual ~RTBufferListener() {}

 public:
    // buffer callback.
    virtual void onBufferAvailable(void* buffer) = 0;
    virtual void onBufferRealloc(void* buffer, UINT32 size) = 0;
    virtual void onBufferRegister(UINT64 poolCapacity, UINT32 size) {}
    virtual void onBufferRelease(void* buffer, RT_BOOL render) = 0;
};

class RTAllocator;
class RTMediaBuffer : public RTObject {
 public:
    // The underlying data remains the responsibility of the caller!
    explicit RTMediaBuffer(void* data, UINT32 size);
    explicit RTMediaBuffer(
                 void* data,
                 UINT32 size,
                 INT32 handle,
                 INT32 fd,
                 RTAllocator *alloctor = RT_NULL);
    explicit RTMediaBuffer(UINT32 size);
    explicit RTMediaBuffer(const RTMediaBuffer* data);
    virtual ~RTMediaBuffer();
    RTMediaBuffer& operator = (const RTMediaBuffer &);

 public:
    // override RTObject methods
    const char* getName() { return "RTMediaBuffer"; }
    void  summary(INT32 fd) { }

 public:
    void   release(bool debug = false);
    void   signalBufferAvailable();
    void   signalBufferRealloc(UINT32 size);
    void   signalBufferRelease(bool render = false);
    void   signalBufferRegister(UINT64 poolCapacity, UINT32 size);

    void*  getData() const;
    UINT32 getSize() const;
    UINT32 getRealSize() const;
    UINT32 getOffset() const;
    UINT32 getLength() const;
    INT32  getFd() const;
    INT32  getHandle() const;
    UINT32 getPhyAddr() const;
    INT32  getUniqueID() const;
    INT32  getBufferSeq() const;
    INT32  getPoolID() const;

    void   setData(void* data, UINT32 size);
    void   setListener(RTBufferListener* listener);
    void   setPhyAddr(UINT32 phyaddr);
    void   setRange(UINT32 offset, UINT32 length);
    void   setRealSize(UINT32 real) { mRealSize = real;}
    void   setStatus(RtMediaBufferStatus status);
    void   setSite(RTMediaBufferSite site);
    void   setPrivateData(void *data);
    void   setBufferID(void *id);
    void   setBufferSeq(INT32 seq);
    void   setFd(INT32 fd);
    void   setHandle(INT32 handle);
    void   setUniqueID(INT32 uniqueId);
    void   setPoolID(INT32 poolId);
    void   setAllocator(RTAllocator *allocator);

    RTAllocator *       getAllocator() { return mAllocator; }
    void*               getPrivateData();
    RtMediaBufferStatus getStatus();
    RtMetaData*         getMetaData();
    RtMetaData*         extraMeta(const INT32 id);
    void                setExtraMeta(RtMetaData *meta, const INT32 id);
    RT_BOOL             hasMeta(const INT32 id) {
        return mExtraMetas.find(id) != mExtraMetas.end();
    }
    RTMediaBufferSite   getSite();
    void*               getBufferID();

    void                setRegistered(RT_BOOL registered);
    RT_BOOL             isRegistered();

    RT_BOOL             isEOS(INT32 id);
    RT_BOOL             isEOS();
    // refs manage
    void                addRefs();
    INT32               refsCount();

    // Clears meta data and resets the range to the full extent.
    void                reset();

 private:
    void                baseInit();
    void                decRefs();

 private:
    void*           mData;
    UINT32          mSize;
    UINT32          mRealSize;
    UINT32          mRangeOffset;
    UINT32          mRangeLength;
    INT32           mHandle;
    INT32           mFd;
    INT32           mUniqueId;    // all process can using this id
    INT32           mPoolID;
    UINT32          mPhyAddr;
    RT_BOOL         mOwnsData;
    INT32           mRefCount;
    void           *mPrivateData;
    void           *mBufferID;
    INT32           mBufferSeq;
    RT_BOOL         mRegistered;
    RTAllocator    *mAllocator;
    RtMutex        *mLock;

    RtMediaBufferStatus     mStatus;
    RTMediaBufferSite       mSite;
    RTBufferListener       *mBufferListener;
    RtMetaData                     *mMetaData;
    std::map<INT32, RtMetaData *>   mExtraMetas;
};

#endif  // SRC_RT_MEDIA_INCLUDE_RTMEDIABUFFER_H_
