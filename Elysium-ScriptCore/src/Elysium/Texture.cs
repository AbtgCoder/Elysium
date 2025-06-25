using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Elysium
{
    public class Texture2D
    {
        public ulong Handle { get; set; }

        protected Texture2D()
        {
            Handle = 0;
        }

        public Texture2D(ulong handle)
        {
            Handle = handle;
        }

        //TODO: load texture asset from filepath

        public int Width => InternalCalls.Texture2D_GetWidth(Handle);
        public int Height => InternalCalls.Texture2D_GetHeight(Handle);

        // helper
        public Vector2 GetWorldSize(float pixelsPerUnit)
        {
            return new Vector2(Width / pixelsPerUnit, Height / pixelsPerUnit);
        }
    }
}
