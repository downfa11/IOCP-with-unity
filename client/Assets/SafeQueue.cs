using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Threading;

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
