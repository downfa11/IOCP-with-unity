using UnityEngine;
using System.Text;

public class PacketManager : MonoBehaviour
{
    public static void Send(int number, object data)
    {
        byte[] data_byte = Manager.StructureToByte(data);

        Packet packet = new Packet();
        packet.number = number;
        packet.data = data_byte;

        Manager.packet_send_queue.Enqueue(packet);
    }
    public static void Send(int number)
    {
        Packet packet = new Packet();
        packet.number = number;
        packet.data = null;

        Manager.packet_send_queue.Enqueue(packet);
    }
    public static void Send(int number, string data)
    {
        if (data.Length > 100)
        {
            data = data.Substring(0, 100);
        }

        byte[] data_byte = Encoding.UTF8.GetBytes(data);

        Packet packet = new Packet();
        packet.number = number;
        packet.data = data_byte;

        Manager.packet_send_queue.Enqueue(packet);
    }
}
