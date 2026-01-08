using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace Elysium
{
    [StructLayout(LayoutKind.Sequential)]
    public struct ContactPoint2D
    {
        public Vector2 Point;
        public Vector2 Normal;
        public float Penetration;
    }


    public class Collision2D
    {
        public Entity Entity { get; internal set; }
        public Entity OtherEntity { get; internal set; }

        public ContactPoint2D[] Contacts { get; internal set; }
    }
}
