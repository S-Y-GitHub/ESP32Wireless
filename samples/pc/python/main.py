from esp32wl import Data, DataType, Wireless
import time

with Wireless() as wl:
    wl.rx_attach(50000, 0)
    wl.rx_attach(50001, 1)
    wl.rx_attach(50002, 2)
    wl.rx_attach(50003, 3)

    wl.tx_attach("192.168.1.1", 50000, 0)
    wl.tx_attach("192.168.1.1", 50001, 1)
    wl.tx_attach("192.168.1.1", 50002, 2)
    wl.tx_attach("192.168.1.1", 50003, 3)

    while True:
        b0 = wl.available(0) > 0
        b1 = wl.available(1) > 0
        b2 = wl.available(2) > 0
        b3 = wl.available(3) > 0

        if b0:
            msg: str = wl.read(0).data
            print(f"received (ch0): {msg}")
        wl.write(Data(DataType.STRING, "Hello. I'm Python"), 0)
        print(f'sent (ch0): "Hello. I\'m Python"')

        if b1:
            v: int = wl.read(1).data
            print(f"received (ch1): {v})")
            wl.write(Data(DataType.UINT32, v + 1), 2)
            print(f"sent (ch2): {v + 1}")
        else:
            wl.write(Data(DataType.UINT32, 0), 2)
            print(f"sent (ch2): 0")

        if b2:
            v: int = wl.read(2).data
            print(f"received (ch2): {v}")
            wl.write(Data(DataType.UINT32, v + 1), 1)
            print(f"sent (ch1): {v + 1}")
        else:
            wl.write(Data(DataType.UINT32, 0), 1)
            print(f"sent (ch1): 0")

        if b3:
            ary: list = wl.read(3).data
            print(
                f"received (ch3): [{ary[0].data}, {ary[1].data}, {ary[2].data}, {ary[3].data}]")

        wl.write(
            Data(DataType.ARRAY, [Data(DataType.BOOL, b) for b in (b0, b1, b2, b3)]), 3)
        print(f"sent (ch3): [{b0}, {b1}, {b2}, {b3}]")
        time.sleep(1.0)
