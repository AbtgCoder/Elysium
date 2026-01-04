using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Elysium;


namespace Sandbox
{
    public class test : Entity
    {
        public float Speed;
        public int testInt = 0;

        public Texture2D groundTexture;
        public Texture2D pipeTexture;
        public Texture2D brickTexture;
        public Texture2D coinTexture;
        public Texture2D platformTexture;

        public float tileSize = 1.0f;
        public int groundHeight = -3;
        public float generationDistance = 20.0f;
        public int segmentWidth = 10;

        private HashSet<Vector2> spawnedPositions = new HashSet<Vector2>();


    }
}
