using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using System;
using System.Threading;
using UnityEngine.Android;
using System.Text;


public class PacketExcute : MonoBehaviour
{
    Packet packet;

    void FixedUpdate()
    {
        PacketExecute();

    }

    void PacketExecute()
    {
        int repeat = Manager.packet_recv_queue.Count;
        for (int i = 0; i < repeat; i++)
        {
            packet = Manager.packet_recv_queue.Dequeue();
            switch (packet.number)
            {
                case PacketNumber.CONNECTSUCCESS:
                    if (Manager.my_socket != 0)
                        return;
                    Manager.my_socket = BitConverter.ToInt32(packet.data, 0);
                    ClientManager.instance.MyClientCreate();
                    break;

                case PacketNumber.CHAT:
                    ChatRecep();
                    break;

                case PacketNumber.H_COORDINATE:
                    if (packet.data == null)
                        return;

                    GetCordinate();

                    break;
                case PacketNumber.GET_NEWBI:
                    if (packet.data == null)
                        return;

                    GetNewbi();

                    break;
                default:

                    Debug.Log("number: " + packet.number+" "+ BitConverter.ToInt32(packet.data, 0));
                    break;
            }
        }
    }


    void ChatRecep()
    {
        if (packet.data == null)
            return;

        byte[] string_byte = new byte[packet.data.Length];

        Array.Copy(packet.data, string_byte, packet.data.Length);


        string chatMessage = Encoding.UTF8.GetString(string_byte);
        ClientManager.instance.ClientChat(chatMessage);
    }

    void GetCordinate()
    {
        if (packet.data == null)
            return;

        string dataString = System.Text.Encoding.UTF8.GetString(packet.data);
        try
        {
            string[] dataParts = dataString.Split(',');
            if (dataParts.Length >= 3)
            {
                int socket = Convert.ToInt32(dataParts[0]);
                int x = Convert.ToInt32(dataParts[1]);
                int y = Convert.ToInt32(dataParts[2]);
                if (socket != Manager.my_socket)
                    ClientManager.instance.ClientMove(socket, x, y);
            }
            else
            {
                Debug.LogError("Invalid data format received from the server: " + dataString);
            }
        }
        catch(FormatException ex)
        {
            Debug.LogError(ex.Message+" : " + dataString);

        }
    }
    void GetNewbi()
    {
        if (packet.data == null)
            return;

        int socket_ = BitConverter.ToInt32(packet.data, 0);
        ClientManager.instance.NewbiCreate(socket_);
    }

    void OnDestroy()
    {
        Manager.Close();
    }
}


