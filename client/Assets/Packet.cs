using System.Runtime.InteropServices;
using System;

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


