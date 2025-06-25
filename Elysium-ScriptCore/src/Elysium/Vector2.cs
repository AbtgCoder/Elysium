using Elysium;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Elysium
{
    public struct Vector2
    {
        public float X, Y;

        public static Vector2 Zero => new Vector2(0, 0);

        public Vector2(float x, float y)
        {
            X = x;
            Y = y;
        }

        public Vector2(float value)
        {
            X = value;
            Y = value;
        }

        public static Vector2 operator +(Vector2 a, Vector2 b)
        {
            return new Vector2(a.X + b.X, a.Y + b.Y);
        }

        public static Vector2 operator -(Vector2 a, Vector2 b)
        {
            return new Vector2(a.X - b.X, a.Y - b.Y);
        }

        public static Vector2 operator *(Vector2 a, float scalar)
        {
            return new Vector2(a.X * scalar, a.Y * scalar);
        }

        public static Vector2 operator /(Vector2 a, float scalar)
        {
            // if (scalar == 0)
                // throw new DivideByZeroException("Cannot divide by zero.");
            return new Vector2(a.X / scalar, a.Y / scalar);
        }
    }
}
