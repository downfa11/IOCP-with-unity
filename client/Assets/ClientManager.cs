using System;
using System.Collections;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using Unity.VisualScripting;
using UnityEngine;
using UnityEngine.UI;

public static class PacketNumber
{
    public const int CONNECTSUCCESS = 1000;
    public const int CHAT = 8282;
    public const int H_COORDINATE = 3142;
    public const int GET_NEWBI = 4910;
}

public class ClientManager : MonoBehaviour
{
    public static ClientManager instance;
    static public LinkedList<Client> client_list = new LinkedList<Client>();
    static public Client my_client;
    int Count = 0;
    float packet_time;

    public Text chatBox;
    public Text curClient;
    public GameObject chatlist;

    public const string IP = "127.0.0.1";
    public const int Port = 8080;

    public GameObject client_prefab;

    public InputField textInput;
    public bool isChat;

    void Awake()
    {
        instance = this;
        packet_time = Time.time;
        client_list.Clear();

        Manager.ConnectServer(IP, Port);
    }
    void Update()
    {
        Count = client_list.Count;
        if (Manager.connection_state == Manager.ConnectionState.Disconnect)
        { curClient.text = "Disconnected.";

            if (Input.GetMouseButtonDown(0))
                Manager.ConnectServer(IP, Port);

        }
        else curClient.text = "Clients : " + Count.ToString();



        if (Manager.connection_state == Manager.ConnectionState.ConnectSuccess)
            if (my_client != null)
                my_client.SpeedDecrease();


        isChat = textInput.isFocused;
        if (Input.GetKeyDown(KeyCode.Return))
        {
            if (textInput.text != "")
            {
                KeySend();
                textInput.ActivateInputField();
            }
            else
            { textInput.Select(); }
        }
    }
    public void ClientChat(string text)
    {
        Text chatText = Instantiate(chatBox, chatlist.transform).GetComponent<Text>();
        chatText.text = text;

    }
    public void ClientMove(int socket, int x, int y)
    {
        foreach (var item in client_list)
        {
            if (item.socket == socket)
            {
                if (item.x == x && item.y == y)
                    return;

                item.x = x;
                item.y = y;
                //item.transform.position = new Vector3(item.x, item.y, 0);
                break;
            }
        }
    }
    public void NewbiCreate(int socket)
    {

        bool isDuplicate = false;
        foreach (var item in client_list)
        {

            if (item.socket == socket)
            {
                isDuplicate = true;
                break;
            }
        }
        if (!isDuplicate)
        {
            Client newbi = Instantiate(instance.client_prefab).GetComponent<Client>();
            newbi.socket = socket;
            newbi.x = 0;
            newbi.y = 0;
            newbi.transform.position = new Vector2(newbi.x, newbi.y);
            client_list.AddLast(newbi);
        }


    }
    public void MyClientCreate()
    {
        my_client = Instantiate(instance.client_prefab).GetComponent<Client>();
        my_client.socket = Manager.my_socket;
        client_list.AddLast(my_client);
        my_client.transform.tag = CameraMovement.targetTag;
        my_client.nameText.color = Color.green;
    }
    public void KeySend()
    {
        string a = textInput.text;
        PacketManager.Send(PacketNumber.CHAT, a);
        textInput.text = "";
    }
}
