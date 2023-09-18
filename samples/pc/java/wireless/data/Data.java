package wireless.data;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class Data implements Cloneable {
    public static final byte TYPE_NULL = 0;
    public static final byte TYPE_TRUE = 1;
    public static final byte TYPE_FALSE = 2;
    public static final byte TYPE_STRING = 3;
    public static final byte TYPE_ARRAY = 4;
    public static final byte TYPE_INT8 = 5;
    public static final byte TYPE_INT16 = 6;
    public static final byte TYPE_INT32 = 7;
    public static final byte TYPE_INT64 = 8;
    public static final byte TYPE_UINT8 = 9;
    public static final byte TYPE_UINT16 = 10;
    public static final byte TYPE_UINT32 = 11;
    public static final byte TYPE_UINTt64 = 12;

    private final DataType type;
    private final Object data;

    private Data(DataType type, Object data) {
        this.type = type;
        this.data = data;
    }

    public Data() {
        this(DataType.Null, null);
    }

    public Data(boolean b) {
        this(DataType.Bool, b);
    }

    public Data(String str) {
        this(DataType.String, str);
    }

    public Data(Data... data) {
        this(DataType.Array, data.clone());
    }

    public Data(byte i8) {
        this(DataType.Int8, i8);
    }

    public Data(short i16) {
        this(DataType.Int16, i16);
    }

    public Data(int i32) {
        this(DataType.Int32, i32);
    }

    public Data(long i64) {
        this(DataType.Int64, i64);
    }

    public static Data asInt8(byte i) {
        return new Data(DataType.Int8, i);
    }

    public static Data asInt16(short i) {
        return new Data(DataType.Int16, i);
    }

    public static Data asInt32(int i) {
        return new Data(DataType.Int32, i);
    }

    public static Data asInt64(long i) {
        return new Data(DataType.Int64, i);
    }

    public static Data asUInt8(byte i) {
        return new Data(DataType.Int8, i);
    }

    public static Data asUInt16(short i) {
        return new Data(DataType.Int16, i);
    }

    public static Data asUInt32(int i) {
        return new Data(DataType.UInt32, i);
    }

    public static Data asUint64(long i) {
        return new Data(DataType.UInt64, i);
    }

    public Object getData() {
        return data;
    }

    private void _serialize(ByteBuffer buffer) {
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        switch (type) {
            case Null -> buffer.put(TYPE_NULL);
            case Bool -> buffer.put((Boolean) data ? TYPE_TRUE : TYPE_FALSE);
            case String -> {
                String str = (String) data;
                buffer.put(TYPE_STRING);
                buffer.putShort((short) str.length());
                buffer.put(str.getBytes());
            }
            case Array -> {
                Data[] array = (Data[]) data;
                buffer.put(TYPE_ARRAY);
                buffer.putShort((short) array.length);
                for (Data a : array)
                    a.serialize(buffer);
            }
            case Int8 -> {
                buffer.put(TYPE_INT8);
                buffer.put((Byte) data);
            }
            case Int16 -> {
                buffer.put(TYPE_INT16);
                buffer.putShort((Short) data);
            }
            case Int32 -> {
                buffer.put(TYPE_INT32);
                buffer.putInt((Integer) data);
            }
            case Int64 -> {
                buffer.put(TYPE_INT64);
                buffer.putLong((Long) data);
            }
            case UInt8 -> {
                buffer.put(TYPE_UINT8);
                buffer.put((Byte) data);
            }
            case UInt16 -> {
                buffer.put(TYPE_UINT16);
                buffer.putShort((Short) data);
            }
            case UInt32 -> {
                buffer.put(TYPE_UINT32);
                buffer.putInt((Integer) data);
            }
            case UInt64 -> {
                buffer.put(TYPE_UINTt64);
                buffer.putLong((Long) data);
            }
        }
    }

    public void serialize(ByteBuffer buffer) {
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        _serialize(buffer);
    }

    private static Data _deserialize(ByteBuffer buffer) {
        Data data;
        switch (buffer.get()) {
            case TYPE_NULL -> data = new Data();
            case TYPE_TRUE -> data = new Data(true);
            case TYPE_FALSE -> data = new Data(false);
            case TYPE_STRING -> {
                int length = Short.toUnsignedInt(buffer.getShort());
                byte[] buf = new byte[length];
                buffer.get(buf);
                data = new Data(new String(buf));
            }
            case TYPE_ARRAY -> {
                int length = Short.toUnsignedInt(buffer.getShort());
                Data[] array = new Data[length];
                for (int i = 0; i < length; ++i)
                    if ((array[i] = _deserialize(buffer)) == null)
                        return null;
                data = new Data(array);
            }
            case TYPE_INT8 -> data = new Data(buffer.get());
            case TYPE_INT16 -> data = new Data(buffer.getShort());
            case TYPE_INT32 -> data = new Data(buffer.getInt());
            case TYPE_INT64 -> data = new Data(buffer.getLong());
            case TYPE_UINT8 -> data = asUInt8(buffer.get());
            case TYPE_UINT16 -> data = asUInt16(buffer.getShort());
            case TYPE_UINT32 -> data = asUInt32(buffer.getInt());
            case TYPE_UINTt64 -> data = asUint64(buffer.getLong());
            default -> data = null;
        }
        return data;
    }

    public static Data deserialize(ByteBuffer buffer) {
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        Data data = _deserialize(buffer);
        return buffer.remaining() == 0 ? data : null;
    }
}
