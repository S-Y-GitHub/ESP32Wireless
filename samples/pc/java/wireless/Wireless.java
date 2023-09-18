package wireless;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.nio.ByteBuffer;
import java.util.ArrayDeque;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Queue;
import java.util.Set;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicBoolean;

import wireless.data.Data;

public class Wireless implements AutoCloseable {
    public static final int MAX_PACKET_SIZE = 1024;

    /** 受信バッファマップ（key: チャンネル, value: 受信バッファ） */
    private final Map<Integer, Queue<Data>> rxBuffers = new HashMap<>();
    /** 受信チャンネルマップ（key: ポート番号, value: チャンネルの集合） */
    private final Map<Integer, Set<Integer>> rxChannels = new HashMap<>();
    /** 送信チャンネルマップ（key: チャンネル, value: アドレスの集合） */
    private final Map<Integer, Set<InetSocketAddress>> txAddresses = new HashMap<>();
    /** スレッドプール */
    private final ExecutorService pool = Executors.newCachedThreadPool();
    /** 受信スレッド実行フラグ */
    private final AtomicBoolean running = new AtomicBoolean(true);
    /** データ送信用ソケット */
    private final DatagramSocket socket;

    public Wireless() throws SocketException {
        socket = new DatagramSocket();
    }

    private void receive(int port) {
        try (DatagramSocket socket = new DatagramSocket(port)) {
            socket.setSoTimeout(1000);
            while (running.get())
                try {
                    byte[] buf = new byte[MAX_PACKET_SIZE];
                    DatagramPacket packet = new DatagramPacket(buf, buf.length);
                    socket.receive(packet);
                    Data data = Data.deserialize(ByteBuffer.wrap(Arrays.copyOf(packet.getData(), packet.getLength())));
                    if (data != null)
                        synchronized (this) {
                            if (rxChannels.containsKey(port))
                                for (int channel : rxChannels.get(port))
                                    if (rxBuffers.containsKey(channel))
                                        rxBuffers.get(channel).add(data);
                                    else {
                                        Queue<Data> rxBuf = new ArrayDeque<>();
                                        rxBuf.add(data);
                                        rxBuffers.put(channel, rxBuf);
                                    }
                        }
                    else
                        System.err.printf("不正なパケットを受信しました。(ポート番号: %d)\n", port);
                } catch (SocketTimeoutException e) {
                    // DO NOTHING
                }
        } catch (IOException e) {
            e.printStackTrace();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public synchronized void rxAttach(int port, int channel) {
        if (rxChannels.containsKey(port))
            rxChannels.get(port).add(channel);
        else {
            Set<Integer> channels = new HashSet<>();
            channels.add(channel);
            rxChannels.put(port, channels);
            pool.submit(() -> receive(port));
        }
    }

    public synchronized int available(int channel) {
        return rxBuffers.containsKey(channel) ? rxBuffers.get(channel).size() : 0;
    }

    public int available() {
        return available(0);
    }

    public synchronized Data read(int channel) {
        return rxBuffers.containsKey(channel) ? rxBuffers.get(channel).poll() : null;
    }

    public synchronized Data read() {
        return read(0);
    }

    public void write(Data data, int channel) throws IOException {
        if (txAddresses.containsKey(channel)) {
            ByteBuffer buffer = ByteBuffer.allocate(MAX_PACKET_SIZE);
            data.serialize(buffer);
            buffer.flip();
            byte[] buf = new byte[buffer.limit()];
            buffer.get(buf);
            for (InetSocketAddress address : txAddresses.get(channel))
                socket.send(new DatagramPacket(buf.clone(), buf.length, address));
        }
    }

    public void write(Data data) throws IOException {
        write(data, 0);
    }

    public void rxAttach(int port) {
        rxAttach(port, 0);
    }

    public synchronized void rxDetach(int port, int channel) {
        if (rxChannels.containsKey(port))
            rxChannels.get(port).remove(channel);
    }

    public void rxDetach(int port) {
        rxDetach(port, 0);
    }

    public void txAttach(InetSocketAddress address, int channel) {
        if (txAddresses.containsKey(channel))
            txAddresses.get(channel).add(address);
        else {
            Set<InetSocketAddress> addresses = new HashSet<>();
            addresses.add(address);
            txAddresses.put(channel, addresses);
        }
    }

    public void txAttach(InetSocketAddress address) {
        txAttach(address, 0);
    }

    public void txAttach(InetAddress address, int port, int channel) {
        txAttach(new InetSocketAddress(address, port), channel);
    }

    public void txAttach(InetAddress address, int port) {
        txAttach(address, port, 0);
    }

    @Override
    public void close() {
        if (running.getAndSet(false))
            pool.shutdown();
    }
}