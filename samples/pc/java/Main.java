import java.io.IOException;
import java.net.InetAddress;

import wireless.Wireless;
import wireless.data.Data;

public class Main {
    public static void main(String[] args) throws InterruptedException {
        try (Wireless wireless = new Wireless()) {
            InetAddress ip = InetAddress.getByAddress(new byte[] { (byte) 192, (byte) 168, (byte) 1, (byte) 1 });
            wireless.txAttach(ip, 50000, 0);
            wireless.txAttach(ip, 50001, 1);
            wireless.txAttach(ip, 50002, 2);
            wireless.txAttach(ip, 50003, 3);
            wireless.rxAttach(50000, 0);
            wireless.rxAttach(50001, 1);
            wireless.rxAttach(50002, 2);
            wireless.rxAttach(50003, 3);

            for (;;) {
                boolean b0, b1, b2, b3;
                if (b0 = wireless.available(0) > 0)
                    System.out.printf("received (ch0): \"%s\"\n", wireless.read(0).getData());
                wireless.write(new Data("Hello. I'm java."));
                System.out.println("sent (ch0): \"Hello. I'm java.");

                if (b1 = wireless.available(1) > 0) {
                    int val = (Integer) wireless.read(1).getData();
                    System.out.printf("received (ch1): %s\n", Integer.toUnsignedString(val));
                    ++val;

                    wireless.write(Data.asUInt32(val), 2);
                    System.out.printf("sent (ch2): %d\n", val);
                } else {
                    wireless.write(Data.asUInt32(0), 2);
                    System.out.println("sent (ch2): 0");
                }
                if (b2 = wireless.available(2) > 0) {
                    int val = (Integer) wireless.read(2).getData();
                    System.out.printf("received (ch2): %s\n", Integer.toUnsignedString(val));
                    ++val;

                    wireless.write(Data.asUInt32(val), 1);
                    System.out.printf("sent (ch1): %d\n", val);
                } else {
                    wireless.write(Data.asUInt32(0), 1);
                    System.out.println("sent (ch1): 0");
                }
                if (b3 = wireless.available(3) > 0) {
                    Data[] data = (Data[]) wireless.read(3).getData();
                    System.out.printf("received (ch3): [%b, %b, %b, %b]\n",
                            data[0].getData(), data[1].getData(), data[2].getData(), data[3].getData());
                }

                wireless.write(new Data(new Data(b0), new Data(b1), new Data(b2), new Data(b3)), 3);
                System.out.printf("sent (ch3): [%b, %b, %b, %b]\n", b0, b1, b2, b3);

                Thread.sleep(1000);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}