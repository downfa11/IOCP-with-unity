using System.Runtime.InteropServices;
using System;
using System.Collections.Generic;
using System.Threading;

public class Packet
{
    public int number = 0;
    public byte[] data = null;
}

[Serializable]
[StructLayout(LayoutKind.Sequential, Pack = 1)]
public struct Header
{
    public int size;
    public int number;
}

public class SafeQueue<T>
{
    private Queue<T> queue = new Queue<T>();
    private Mutex mutex = new Mutex();

    public int Count
    {
        get
        {
            mutex.WaitOne();
            int count = queue.Count;
            mutex.ReleaseMutex();
            return count;
        }
    }

    public void Enqueue(T data)
    {
        mutex.WaitOne();
        queue.Enqueue(data);
        mutex.ReleaseMutex();
    }

    public T Dequeue()
    {
        mutex.WaitOne();
        T data = queue.Dequeue();
        mutex.ReleaseMutex();
        return data;
    }
}

