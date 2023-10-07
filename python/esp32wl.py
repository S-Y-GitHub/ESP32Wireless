from collections import deque
from concurrent.futures import ThreadPoolExecutor
from enum import Enum, auto
from socket import socket, AF_INET, SOCK_DGRAM
from threading import Lock


# データ型
class DataType(Enum):
    NULL = auto()
    BOOL = auto()
    STRING = auto()
    ARRAY = auto()
    INT8 = auto()
    INT16 = auto()
    INT32 = auto()
    INT64 = auto()
    UINT8 = auto()
    UINT16 = auto()
    UINT32 = auto()
    UINT64 = auto()


TYPE_NULL: int = 0
TYPE_TRUE: int = 1
TYPE_FALSE: int = 2
TYPE_STRING: int = 3
TYPE_ARRAY: int = 4
TYPE_INT8: int = 5
TYPE_INT16: int = 6
TYPE_INT32: int = 7
TYPE_INT64: int = 8
TYPE_UINT8: int = 9
TYPE_UINT16: int = 10
TYPE_UINT32: int = 11
TYPE_UINT64: int = 12


class Data:
    def __init__(self, type: DataType = DataType.NULL, data: object = None) -> None:
        self.__type: DataType = type
        self.__data: object = data

    @property
    def type(self):
        return self.__type

    @property
    def data(self):
        return self.__data

    @staticmethod
    def __deserialize(b: bytearray):
        if len(b) == 0:
            return None
        t: int = b.pop(0)
        if t == TYPE_NULL:
            return Data(DataType.NULL, None)
        elif t == TYPE_TRUE:
            return Data(DataType.BOOL, True)
        elif t == TYPE_FALSE:
            return Data(DataType.BOOL, False)
        elif t == TYPE_STRING:
            if len(b) < 2:
                return None
            l = int.from_bytes(b[:2], byteorder="little", signed=False)
            del b[:2]
            if len(b) < l:
                return None
            s: str = b[:l].decode()
            del b[:l]
            return Data(DataType.STRING, s)
        elif t == TYPE_ARRAY:
            if len(b) < 2:
                return None
            l = int.from_bytes(b[:2], byteorder="little", signed=False)
            del b[:2]
            ary: list = list()
            for _ in range(l):
                elem = Data.__deserialize(b)
                if elem is None:
                    return None
                ary.append(elem)
            return Data(DataType.ARRAY, ary)
        elif t == TYPE_INT8:
            if len(b) < 1:
                return None
            i: int = int.from_bytes(b[:1], byteorder="little", signed=True)
            del b[:1]
            return Data(DataType.INT8, i)
        elif t == TYPE_INT16:
            if len(b) < 2:
                return None
            i: int = int.from_bytes(b[:2], byteorder="little", signed=True)
            del b[:2]
            return Data(DataType.INT16, i)
        elif t == TYPE_INT32:
            if len(b) < 4:
                return None
            i: int = int.from_bytes(b[:4], byteorder="little", signed=True)
            del b[:4]
            return Data(DataType.INT32, i)
        elif t == TYPE_INT64:
            if len(b) < 8:
                return None
            i: int = int.from_bytes(b[:8], byteorder="little", signed=True)
            del b[:8]
            return Data(DataType.INT64, i)
        elif t == TYPE_UINT8:
            if len(b) < 1:
                return None
            i: int = int.from_bytes(b[:1], byteorder="little", signed=False)
            del b[:1]
            return Data(DataType.UINT8, i)
        elif t == TYPE_UINT16:
            if len(b) < 2:
                return None
            i: int = int.from_bytes(b[:2], byteorder="little", signed=False)
            del b[:2]
            return Data(DataType.UINT16, i)
        elif t == TYPE_UINT32:
            if len(b) < 4:
                return None
            i: int = int.from_bytes(b[:4], byteorder="little", signed=False)
            del b[:4]
            return Data(DataType.UINT32, i)
        elif t == TYPE_UINT64:
            if len(b) < 8:
                return None
            i: int = int.from_bytes(b[:8], byteorder="little", signed=False)
            del b[:8]
            return Data(DataType.UINT64, i)
        return None

    @staticmethod
    def deserialize(b: bytes):
        bary = bytearray(b)
        data: Data = Data.__deserialize(bary)
        return data if len(bary) == 0 else None

    def __bytes__(self):
        bs: bytearray = bytearray()
        if self.__type is DataType.NULL:
            bs.append(TYPE_NULL)
        elif self.__type is DataType.BOOL:
            bs.append(TYPE_TRUE if self.__data else TYPE_FALSE)
        elif self.__type is DataType.STRING:
            bs.append(TYPE_STRING)
            self.__data: str
            bs += len(self.__data).to_bytes(2,
                                            byteorder="little", signed=False)
            bs += self.__data.encode()
        elif self.__type is DataType.ARRAY:
            bs.append(TYPE_ARRAY)
            self.__data: list
            bs += len(self.__data).to_bytes(2,
                                            byteorder="little", signed=False)
            for d in self.__data:
                bs += bytes(d)
        elif self.__type is DataType.INT8:
            bs.append(TYPE_INT8)
            self.__data: int
            bs += self.__data.to_bytes(1, byteorder="little", signed=True)
        elif self.__type is DataType.INT16:
            bs.append(TYPE_INT16)
            self.__data: int
            bs += self.__data.to_bytes(2, byteorder="little", signed=True)
        elif self.__type is DataType.INT32:
            bs.append(TYPE_INT32)
            self.__data: int
            bs += self.__data.to_bytes(4, byteorder="little", signed=True)
        elif self.__type is DataType.INT64:
            bs.append(TYPE_INT64)
            self.__data: int
            bs += self.__data.to_bytes(8, byteorder="little", signed=True)
        elif self.__type is DataType.UINT8:
            bs.append(TYPE_UINT8)
            self.__data: int
            bs += self.__data.to_bytes(1, byteorder="little", signed=False)
        elif self.__type is DataType.UINT16:
            bs.append(TYPE_UINT16)
            self.__data: int
            bs += self.__data.to_bytes(2, byteorder="little", signed=False)
        elif self.__type is DataType.UINT32:
            bs.append(TYPE_UINT32)
            self.__data: int
            bs += self.__data.to_bytes(4, byteorder="little", signed=False)
        elif self.__type is DataType.UINT64:
            bs.append(TYPE_UINT64)
            self.__data: int
            bs += self.__data.to_bytes(8, byteorder="little", signed=False)
        else:
            raise ValueError()
        return bytes(bs)

    def __bool__(self) -> bool:
        return self.__type is DataType.BOOL and self.__data

    def __str__(self) -> str:
        return self.data if self.__type is DataType.STRING else ""

    def __int__(self) -> int:
        return (
            self.data
            if (
                self.__type is DataType.INT8
                or self.__type is DataType.INT16
                or self.__type is DataType.INT32
                or self.__type is DataType.INT64
                or self.__type is DataType.UINT8
                or self.__type is DataType.UINT16
                or self.__type is DataType.UINT32
                or self.__type is DataType.UINT64
            )
            else -1
        )

    def __iter__(self):
        return self.data if self.__type is DataType.ARRAY else list()


MAX_PACKET_SIZE: int = 256


class Wireless:
    def __init__(self) -> None:
        self.__rx_flag: bool = False
        self.__rx_thread_pool: ThreadPoolExecutor = None
        self.__lock: Lock = None
        self.__socket: socket = None
        self.__rx_bufs: dict = dict()
        self.__tx_channels: dict = dict()
        self.__rx_channels: dict = dict()

    def tx_attach(self, ip: str, port: int, channel: int) -> None:
        adr: tuple = (ip, port)
        if channel in self.__tx_channels.keys():
            if not adr in self.__tx_channels[channel]:
                self.__tx_channels[channel].add(adr)
        else:
            self.__tx_channels[channel] = {
                adr,
            }

    def tx_detach(self, ip: str, port: int, channel: int) -> None:
        if channel in self.__tx_channels.keys():
            self.__tx_channels[channel].discard((ip, port))

    def __rx_loop(self, port: int) -> None:
        s: socket = socket(AF_INET, SOCK_DGRAM)
        s.bind(("0.0.0.0", port))
        while self.__rx_flag:
            b, _ = s.recvfrom(MAX_PACKET_SIZE)
            data = Data.deserialize(b)
            with self.__lock:
                if port in self.__rx_channels.keys():
                    channels: set = self.__rx_channels[port]
                    for channel in channels:
                        buf: deque
                        if channel in self.__rx_bufs:
                            buf = self.__rx_bufs[channel]
                        else:
                            buf = self.__rx_bufs[channel] = deque()
                        buf.append(data)

        s.close()

    def rx_attach(self, port: int, channel: int) -> None:
        with self.__lock:
            if port in self.__rx_channels.keys():
                self.__rx_channels[port].add(channel)
            else:
                self.__rx_channels[port] = {
                    channel,
                }
                self.__rx_thread_pool.submit(self.__rx_loop, port)

    def rx_detach(self, port: int, channel: int) -> None:
        with self.__lock:
            if port in self.__rx_channels.keys():
                self.__rx_channels[port].discard(channel)

    def read(self, channel: int = 0) -> Data:
        with self.__lock:
            if channel in self.__rx_bufs.keys():
                rx_buf: deque = self.__rx_bufs[channel]
                if len(rx_buf) >= 1:
                    return rx_buf.popleft()
        return None

    def available(self, channel: int = 0) -> int:
        with self.__lock:
            if channel in self.__rx_bufs.keys():
                return len(self.__rx_bufs[channel])
        return 0

    def write(self, data: Data, channel: int = 0):
        if channel in self.__tx_channels.keys():
            b: bytes = bytes(data)
            for adr in self.__tx_channels[channel]:
                self.__socket.sendto(b, adr)

    def __enter__(self):
        self.__socket = socket(AF_INET, SOCK_DGRAM)
        self.__lock = Lock()
        self.__rx_flag = True
        self.__rx_thread_pool = ThreadPoolExecutor()
        return self

    def __exit__(self, ex_type, ex_value, trace) -> None:
        self.__rx_flag = False
        self.__socket.close()
        self.__rx_bufs.clear()
        self.__rx_channels.clear()
        self.__rx_thread_pool.shutdown()
        self.__tx_channels.clear()
        self.__lock = None
