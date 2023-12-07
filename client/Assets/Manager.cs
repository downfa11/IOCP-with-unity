using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Net;
using System.Net.Sockets;
using System;
using System.Threading;
using System.Runtime.InteropServices;

public class Manager
{
    public static TcpClient client;
    public static NetworkStream stream;
    public static int my_socket;
    public static int byte_spend;
    public enum ConnectionState
    {
        ConnectTry, ConnectSuccess, Disconnect
    }
    static public ConnectionState connection_state = ConnectionState.Disconnect;
    static public string connection_state_string = "click";

    public static Thread recv_thread;
    public static Thread send_thread;
    public static SafeQueue<Packet> packet_recv_queue = new SafeQueue<Packet>();
    public static SafeQueue<Packet> packet_send_queue = new SafeQueue<Packet>();

    private static string server_ip;
    private static int server_port;

    public static Mutex mutex = new Mutex();

    static public void ConnectServer(string ip, int port)
    {
        if (connection_state == ConnectionState.ConnectSuccess)
        {
            UnityEngine.Debug.Log("연결할 준비가 되지않았습니다");
            return;
        }

        if (connection_state == ConnectionState.ConnectTry)
            return;

        connection_state = ConnectionState.ConnectTry;
        connection_state_string = "connect trying..";

        server_ip = ip;
        server_port = port;

        Thread accept_thread = new Thread(new ThreadStart(ConnectThread));
        accept_thread.Start();
    }

    static void ConnectThread()
    {
        try
        {
            IPEndPoint client_address = new IPEndPoint(0, 0);
            IPEndPoint server_address = new IPEndPoint(IPAddress.Parse(server_ip), server_port);

            client = new TcpClient(client_address);
            client.Connect(server_address);
            stream = client.GetStream();
            client.NoDelay = true;
            client.Client.NoDelay = true;

            connection_state_string = "connection success";
            connection_state = ConnectionState.ConnectSuccess;

            recv_thread = new Thread(new ThreadStart(RecvThread));
            recv_thread.Start();

            send_thread = new Thread(new ThreadStart(SendThread));
            send_thread.Start();
        }
        catch (Exception ex)
        {
            connection_state = ConnectionState.Disconnect;
            connection_state_string = ex.ToString();
            UnityEngine.Debug.Log(ex);
        }

        return;
    }

    static void SendThread()
    {
        while (true)
        {
            if (connection_state == ConnectionState.Disconnect)
                return;

            int count = packet_send_queue.Count;

            if (count != 0)
            {
                for (int i = 0; i < count; i++)
                {
                    Packet packet = packet_send_queue.Dequeue();
                    Header header = new Header();

                    header.number = packet.number;

                    if (packet.data != null)
                        header.size = packet.data.Length;
                    else
                        header.size = 0;

                    byte[] header_byte = StructureToByte(header);
                    byte[] result_byte;

                    if (packet.data != null)
                    {
                        header.size = packet.data.Length;
                        result_byte = new byte[header_byte.Length + packet.data.Length];

                        Array.Copy(header_byte, 0, result_byte, 0, header_byte.Length);
                        Array.Copy(packet.data, 0, result_byte, header_byte.Length, packet.data.Length);
                    }
                    else
                    {
                        header.size = 0;
                        result_byte = header_byte;
                    }
                    stream.Write(result_byte, 0, result_byte.Length);
                    stream.Flush();
                    byte_spend += result_byte.Length;

                }
            }
            Thread.Sleep(1);
        }
    }



    static void RecvThread()
    {
        while (true)
        {
            if (connection_state == ConnectionState.Disconnect)
                return;
            try
            {
                byte[] lengthBytes = new byte[sizeof(int)];
                stream.Read(lengthBytes, 0, sizeof(int));
                int size = BitConverter.ToInt32(lengthBytes, 0);

                byte[] numberBytes = new byte[sizeof(int)];
                stream.Read(numberBytes, 0, sizeof(int));
                int number = BitConverter.ToInt32(numberBytes, 0);


                Packet packet = new Packet();
                packet.number = number;

                if (size != 0)
                {
                    packet.data = new byte[size];
                    stream.Read(packet.data, 0, size);
                }

                byte_spend += sizeof(int) * 2 + size;

                packet_recv_queue.Enqueue(packet);
            }
            catch (ObjectDisposedException ex) { Debug.LogError(ex); }
        }
    }


    public static void Close()
    {
        stream.Close();
        client.Close();

        connection_state = ConnectionState.Disconnect;

        send_thread.Join();
        recv_thread.Interrupt();
        recv_thread.Abort();
    }

    public static byte[] StructureToByte(object obj)
    {
        int datasize = Marshal.SizeOf(obj); // 구조체에 할당된 메모리의 크기를 구한다.
        byte[] data = new byte[datasize]; // 구조체가 복사될 배열
        IntPtr buff = Marshal.AllocHGlobal(datasize); // 비관리 메모리 영역에 구조체 크기만큼의 메모리를 할당한다.


        try
        {
            Marshal.StructureToPtr(obj, buff, false); // 할당된 구조체 객체의 주소를 구한다.
            Marshal.Copy(buff, data, 0, datasize); // 구조체 객체를 배열에 복사
        }
        finally
        {
            Marshal.FreeHGlobal(buff); // 비관리 메모리 영역에 할당했던 메모리를 해제함
        }

        return data;
    }

}
