using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
public class Client : MonoBehaviour
{
    public int socket;
    public int x;
    public int y;

    public float speed = 2;

    public float vspeed = 0;
    public float hspeed = 0;
    float max_speed = 10f;

    private Vector3 previousPosition;
    public Text nameText;


    private void Start()
    {
        previousPosition = transform.position;
    }

    private void Update()
    {
        nameText.text = socket.ToString();


        if (socket == Manager.my_socket)
        {
            hspeed = Input.GetAxis("Horizontal");
            vspeed = Input.GetAxis("Vertical");

            transform.position += new Vector3((float)hspeed, (float)vspeed, 0) * speed * Time.deltaTime;

            Vector3 newPosition = transform.position + new Vector3(hspeed, vspeed, 0) * speed * Time.deltaTime;

            if (Mathf.Abs(newPosition.x - previousPosition.x) > 1 || Mathf.Abs(newPosition.y - previousPosition.y) > 1)
            {

                string positionString = $"{(int)newPosition.x},{(int)newPosition.y}";
                PacketManager.Send(PacketNumber.H_COORDINATE, positionString);
                previousPosition = newPosition;
            }
        }
        else transform.position += new Vector3((float)hspeed, (float)vspeed, 0) * speed * Time.deltaTime;
    }

    void FixedUpdate()
    {
        if (socket != Manager.my_socket)
        {
            SpeedDecrease();
            if (transform.position != new Vector3(x, y, 0))
            {
                Vector2 targetDirection = new Vector3(x, y, 0) - transform.position;
                if (targetDirection.sqrMagnitude > 0.5f)
                    Run(targetDirection.normalized);


            }


        }
    }

    public void Run(Vector3 direction)
    {
        float plusSpeed = 0.03f;

        if (direction.x > 0)
        {
            if (hspeed < direction.x * max_speed)
            {
                hspeed += direction.x * plusSpeed;
                if (hspeed > direction.x * max_speed)
                    hspeed = direction.x * max_speed;
            }
        }
        else
        {
            if (hspeed > direction.x * max_speed)
            {
                hspeed += direction.x * plusSpeed;
                if (hspeed < direction.x * max_speed)
                    hspeed = direction.x * max_speed;
            }
        }

        if (direction.y > 0)
        {
            if (vspeed < direction.y * max_speed)
            {
                vspeed += direction.y * plusSpeed;
                if (vspeed > direction.y * max_speed)
                    vspeed = direction.y * max_speed;
            }
        }
        else
        {
            if (vspeed > direction.y * max_speed)
            {
                vspeed += direction.y * plusSpeed;
                if (vspeed < direction.y * max_speed)
                    vspeed = direction.y * max_speed;
            }
        }
    }

    public void SpeedDecrease()
    {
        float decSpeed = 0.02f;

        Vector2 n = new Vector2(hspeed, vspeed).normalized;

        if (hspeed > 0)
        {
            hspeed -= n.x * decSpeed;
            if (hspeed < 0)
                hspeed = 0;
        }
        if (hspeed < 0)
        {
            hspeed -= n.x * decSpeed;
            if (hspeed > 0)
                hspeed = 0;
        }
        if (vspeed > 0)
        {
            vspeed -= n.y * decSpeed;
            if (vspeed < 0)
                vspeed = 0;
        }
        if (vspeed < 0)
        {
            vspeed -= n.y * decSpeed;
            if (vspeed > 0)
                vspeed = 0;
        }




        if (Mathf.Abs(hspeed) < 0.001f)
        {
            hspeed = 0;
        }
        if (Mathf.Abs(vspeed) < 0.001f)
        {
            vspeed = 0;
        }

    }

}