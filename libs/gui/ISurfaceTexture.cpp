/*
 * Copyright (C) 2010 The Android Open Source Project
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
 */

#include <stdint.h>
#include <sys/types.h>

#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/Vector.h>
#include <utils/Timers.h>

#include <binder/Parcel.h>
#include <binder/IInterface.h>

#include <gui/ISurfaceTexture.h>

namespace android {
// ----------------------------------------------------------------------------

enum {
    REQUEST_BUFFER = IBinder::FIRST_CALL_TRANSACTION,
    SET_BUFFER_COUNT,
    DEQUEUE_BUFFER,
    QUEUE_BUFFER,
    CANCEL_BUFFER,
    QUERY,
    SET_SYNCHRONOUS_MODE,
    CONNECT,
    DISCONNECT,
#ifdef OMAP_ENHANCEMENT_CPCAM
    UPDATE_AND_GET_CURRENT,
    ADD_BUFFER_SLOT,
    GET_ID,
    RELEASE_BUFFER
#endif
};


class BpSurfaceTexture : public BpInterface<ISurfaceTexture>
{
public:
    BpSurfaceTexture(const sp<IBinder>& impl)
        : BpInterface<ISurfaceTexture>(impl)
    {
    }

    virtual status_t requestBuffer(int bufferIdx, sp<GraphicBuffer>* buf) {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceTexture::getInterfaceDescriptor());
        data.writeInt32(bufferIdx);
        status_t result =remote()->transact(REQUEST_BUFFER, data, &reply);
        if (result != NO_ERROR) {
            return result;
        }
        bool nonNull = reply.readInt32();
        if (nonNull) {
            *buf = new GraphicBuffer();
            reply.read(**buf);
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t setBufferCount(int bufferCount)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceTexture::getInterfaceDescriptor());
        data.writeInt32(bufferCount);
        status_t result =remote()->transact(SET_BUFFER_COUNT, data, &reply);
        if (result != NO_ERROR) {
            return result;
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t dequeueBuffer(int *buf, sp<Fence>& fence,
            uint32_t w, uint32_t h, uint32_t format, uint32_t usage) {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceTexture::getInterfaceDescriptor());
        data.writeInt32(w);
        data.writeInt32(h);
        data.writeInt32(format);
        data.writeInt32(usage);
        status_t result = remote()->transact(DEQUEUE_BUFFER, data, &reply);
        if (result != NO_ERROR) {
            return result;
        }
        *buf = reply.readInt32();
        fence.clear();
        bool hasFence = reply.readInt32();
        if (hasFence) {
            fence = new Fence();
            reply.read(*fence.get());
        }
        result = reply.readInt32();
        return result;
    }

#ifdef OMAP_ENHANCEMENT_CPCAM
    virtual status_t queueBuffer(int buf,
            const QueueBufferInput& input, QueueBufferOutput* output,
            const sp<IMemory>& metadata) {
#else
    virtual status_t queueBuffer(int buf,
            const QueueBufferInput& input, QueueBufferOutput* output) {
#endif
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceTexture::getInterfaceDescriptor());
        data.writeInt32(buf);
        data.write(input);
#ifdef OMAP_ENHANCEMENT_CPCAM
        data.writeStrongBinder(metadata->asBinder());
#endif
        status_t result = remote()->transact(QUEUE_BUFFER, data, &reply);
        if (result != NO_ERROR) {
            return result;
        }
        memcpy(output, reply.readInplace(sizeof(*output)), sizeof(*output));
        result = reply.readInt32();
        return result;
    }

    virtual void cancelBuffer(int buf, sp<Fence> fence) {
        Parcel data, reply;
        bool hasFence = fence.get() && fence->isValid();
        data.writeInterfaceToken(ISurfaceTexture::getInterfaceDescriptor());
        data.writeInt32(buf);
        data.writeInt32(hasFence);
        if (hasFence) {
            data.write(*fence.get());
        }
        remote()->transact(CANCEL_BUFFER, data, &reply);
    }

    virtual int query(int what, int* value) {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceTexture::getInterfaceDescriptor());
        data.writeInt32(what);
        status_t result = remote()->transact(QUERY, data, &reply);
        if (result != NO_ERROR) {
            return result;
        }
        value[0] = reply.readInt32();
        result = reply.readInt32();
        return result;
    }

    virtual status_t setSynchronousMode(bool enabled) {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceTexture::getInterfaceDescriptor());
        data.writeInt32(enabled);
        status_t result = remote()->transact(SET_SYNCHRONOUS_MODE, data, &reply);
        if (result != NO_ERROR) {
            return result;
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t connect(int api, QueueBufferOutput* output) {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceTexture::getInterfaceDescriptor());
        data.writeInt32(api);
        status_t result = remote()->transact(CONNECT, data, &reply);
        if (result != NO_ERROR) {
            return result;
        }
        memcpy(output, reply.readInplace(sizeof(*output)), sizeof(*output));
        result = reply.readInt32();
        return result;
    }

    virtual status_t disconnect(int api) {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceTexture::getInterfaceDescriptor());
        data.writeInt32(api);
        status_t result =remote()->transact(DISCONNECT, data, &reply);
        if (result != NO_ERROR) {
            return result;
        }
        result = reply.readInt32();
        return result;
    }

#ifdef OMAP_ENHANCEMENT_CPCAM
    virtual status_t updateAndGetCurrent(sp<GraphicBuffer>* buf, int &slot) {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceTexture::getInterfaceDescriptor());
        status_t result = remote()->transact(UPDATE_AND_GET_CURRENT, data, &reply);
        if (result != NO_ERROR) {
            return result;
        }
        bool nonNull = reply.readInt32();
        if (nonNull) {
            *buf = new GraphicBuffer();
            reply.read(**buf);
            slot = reply.readInt32();
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t releaseBuffer(int slot) {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceTexture::getInterfaceDescriptor());
        data.writeInt32(slot);
        status_t result = remote()->transact(RELEASE_BUFFER, data, &reply);
        if (result != NO_ERROR) {
            return result;
        }
        result = reply.readInt32();
        return result;
    }

    virtual int addBufferSlot(const sp<GraphicBuffer>& buffer) {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceTexture::getInterfaceDescriptor());
        data.write(*buffer);
        status_t result = remote()->transact(ADD_BUFFER_SLOT, data, &reply);
        if (result != NO_ERROR) {
            return -1;
        }
        int bufferIndex = reply.readInt32();
        return bufferIndex;
    }

    virtual String8 getId() const {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceTexture::getInterfaceDescriptor());
        status_t result =remote()->transact(GET_ID, data, &reply);
        String8 id = reply.readString8();
        return id;
    }
#endif
};

IMPLEMENT_META_INTERFACE(SurfaceTexture, "android.gui.SurfaceTexture");

// ----------------------------------------------------------------------

status_t BnSurfaceTexture::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case REQUEST_BUFFER: {
            CHECK_INTERFACE(ISurfaceTexture, data, reply);
            int bufferIdx   = data.readInt32();
            sp<GraphicBuffer> buffer;
            int result = requestBuffer(bufferIdx, &buffer);
            reply->writeInt32(buffer != 0);
            if (buffer != 0) {
                reply->write(*buffer);
            }
            reply->writeInt32(result);
            return NO_ERROR;
        } break;
        case SET_BUFFER_COUNT: {
            CHECK_INTERFACE(ISurfaceTexture, data, reply);
            int bufferCount = data.readInt32();
            int result = setBufferCount(bufferCount);
            reply->writeInt32(result);
            return NO_ERROR;
        } break;
        case DEQUEUE_BUFFER: {
            CHECK_INTERFACE(ISurfaceTexture, data, reply);
            uint32_t w      = data.readInt32();
            uint32_t h      = data.readInt32();
            uint32_t format = data.readInt32();
            uint32_t usage  = data.readInt32();
            int buf;
            sp<Fence> fence;
            int result = dequeueBuffer(&buf, fence, w, h, format, usage);
            bool hasFence = fence.get() && fence->isValid();
            reply->writeInt32(buf);
            reply->writeInt32(hasFence);
            if (hasFence) {
                reply->write(*fence.get());
            }
            reply->writeInt32(result);
            return NO_ERROR;
        } break;
        case QUEUE_BUFFER: {
            CHECK_INTERFACE(ISurfaceTexture, data, reply);
            int buf = data.readInt32();
            QueueBufferInput input(data);
            QueueBufferOutput* const output =
                    reinterpret_cast<QueueBufferOutput *>(
                            reply->writeInplace(sizeof(QueueBufferOutput)));
#ifdef OMAP_ENHANCEMENT_CPCAM
            sp<IMemory> metadata = interface_cast<IMemory>(data.readStrongBinder());
            status_t result = queueBuffer(buf, input, output, metadata);
#else
            status_t result = queueBuffer(buf, input, output);
#endif
            reply->writeInt32(result);
            return NO_ERROR;
        } break;
        case CANCEL_BUFFER: {
            CHECK_INTERFACE(ISurfaceTexture, data, reply);
            int buf = data.readInt32();
            sp<Fence> fence;
            bool hasFence = data.readInt32();
            if (hasFence) {
                fence = new Fence();
                data.read(*fence.get());
            }
            cancelBuffer(buf, fence);
            return NO_ERROR;
        } break;
        case QUERY: {
            CHECK_INTERFACE(ISurfaceTexture, data, reply);
            int value;
            int what = data.readInt32();
            int res = query(what, &value);
            reply->writeInt32(value);
            reply->writeInt32(res);
            return NO_ERROR;
        } break;
        case SET_SYNCHRONOUS_MODE: {
            CHECK_INTERFACE(ISurfaceTexture, data, reply);
            bool enabled = data.readInt32();
            status_t res = setSynchronousMode(enabled);
            reply->writeInt32(res);
            return NO_ERROR;
        } break;
        case CONNECT: {
            CHECK_INTERFACE(ISurfaceTexture, data, reply);
            int api = data.readInt32();
            QueueBufferOutput* const output =
                    reinterpret_cast<QueueBufferOutput *>(
                            reply->writeInplace(sizeof(QueueBufferOutput)));
            status_t res = connect(api, output);
            reply->writeInt32(res);
            return NO_ERROR;
        } break;
        case DISCONNECT: {
            CHECK_INTERFACE(ISurfaceTexture, data, reply);
            int api = data.readInt32();
            status_t res = disconnect(api);
            reply->writeInt32(res);
            return NO_ERROR;
        } break;

#ifdef OMAP_ENHANCEMENT_CPCAM
        case UPDATE_AND_GET_CURRENT: {
            CHECK_INTERFACE(ISurfaceTexture, data, reply);
            sp<GraphicBuffer> buffer;
            int slot;
            int result = updateAndGetCurrent(&buffer, slot);
            reply->writeInt32(buffer != 0);
            if (buffer != 0) {
                reply->write(*buffer);
                reply->writeInt32(slot);
            }
            reply->writeInt32(result);
            return NO_ERROR;
        } break;
       case ADD_BUFFER_SLOT: {
            CHECK_INTERFACE(ISurfaceTexture, data, reply);
            sp<GraphicBuffer> buffer = new GraphicBuffer();
            data.read(*buffer);
            int bufferIndex = addBufferSlot(buffer);
            reply->writeInt32(bufferIndex);
            return NO_ERROR;
        } break;
       case GET_ID: {
            CHECK_INTERFACE(ISurfaceTexture, data, reply);
            String8 result = getId();
            reply->writeString8(result);
            return NO_ERROR;
        } break;
       case RELEASE_BUFFER: {
            CHECK_INTERFACE(ISurfaceTexture, data, reply);
            int slot = data.readInt32();
            int result = releaseBuffer(slot);
            reply->writeInt32(result);
            return NO_ERROR;
        } break;
#endif
    }
    return BBinder::onTransact(code, data, reply, flags);
}

// ----------------------------------------------------------------------------

static bool isValid(const sp<Fence>& fence) {
    return fence.get() && fence->isValid();
}

ISurfaceTexture::QueueBufferInput::QueueBufferInput(const Parcel& parcel) {
    parcel.read(*this);
}

size_t ISurfaceTexture::QueueBufferInput::getFlattenedSize() const
{
    return sizeof(timestamp)
         + sizeof(crop)
         + sizeof(scalingMode)
         + sizeof(transform)
         + sizeof(bool)
         + (isValid(fence) ? fence->getFlattenedSize() : 0);
}

size_t ISurfaceTexture::QueueBufferInput::getFdCount() const
{
    return isValid(fence) ? fence->getFdCount() : 0;
}

status_t ISurfaceTexture::QueueBufferInput::flatten(void* buffer, size_t size,
        int fds[], size_t count) const
{
    status_t err = NO_ERROR;
    bool haveFence = isValid(fence);
    char* p = (char*)buffer;
    memcpy(p, &timestamp,   sizeof(timestamp));   p += sizeof(timestamp);
    memcpy(p, &crop,        sizeof(crop));        p += sizeof(crop);
    memcpy(p, &scalingMode, sizeof(scalingMode)); p += sizeof(scalingMode);
    memcpy(p, &transform,   sizeof(transform));   p += sizeof(transform);
    memcpy(p, &haveFence,   sizeof(haveFence));   p += sizeof(haveFence);
    if (haveFence) {
        err = fence->flatten(p, size - (p - (char*)buffer), fds, count);
    }
    return err;
}

status_t ISurfaceTexture::QueueBufferInput::unflatten(void const* buffer,
        size_t size, int fds[], size_t count)
{
    status_t err = NO_ERROR;
    bool haveFence;
    const char* p = (const char*)buffer;
    memcpy(&timestamp,   p, sizeof(timestamp));   p += sizeof(timestamp);
    memcpy(&crop,        p, sizeof(crop));        p += sizeof(crop);
    memcpy(&scalingMode, p, sizeof(scalingMode)); p += sizeof(scalingMode);
    memcpy(&transform,   p, sizeof(transform));   p += sizeof(transform);
    memcpy(&haveFence,   p, sizeof(haveFence));   p += sizeof(haveFence);
    if (haveFence) {
        fence = new Fence();
        err = fence->unflatten(p, size - (p - (const char*)buffer), fds, count);
    }
    return err;
}

}; // namespace android
