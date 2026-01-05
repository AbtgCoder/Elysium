using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Elysium;

public class LevelGenerator : Entity
{
    public Texture2D groundTexture;
    public Texture2D pipeTexture;
    public Texture2D brickTexture;
    public Texture2D coinTexture;
    public Texture2D platformTexture;

    public float tileSize = 1.0f;
    public int groundHeight = -3;
    public float generationDistance = 20.0f;
    public int segmentWidth = 10;

    private float lastX = 0;

    private TransformComponent _playerTransform;

    private float groundTextureWidth = 1; // Width of the ground texture in world units
    private float groundTextureHeight = 1; // Height of the ground texture in world units
    private enum SegmentType
    {
        Flat,
        Gaps,
        Stairs,
        Platforms,
        Coins
    }

    void OnCreate()
    {
        Console.WriteLine($"LevelGenerator.OnCreate - {ID}");
        _playerTransform = FindEntityByName("Player").GetComponent<TransformComponent>();

        if (_playerTransform == null)
        {
            Console.WriteLine("Player transform not found. Level generation will not work properly.");
            return;
        }
        lastX = _playerTransform.Position.X - generationDistance;

        groundTextureWidth = groundTexture.GetWorldSize(100).X;
        groundTextureHeight = groundTexture.GetWorldSize(100).Y;
    }

    void OnUpdate(float deltaTime)
    {
        if (_playerTransform == null)
        {
            return;
        }
        //Console.WriteLine($"player pos: {_playerTransform.Position.X}, {_playerTransform.Position.Y}");

        float targetX = _playerTransform.Position.X + generationDistance;

        while (lastX < targetX)
        {
            GenerateSegment(lastX, segmentWidth);
            lastX += segmentWidth;
        }

    }

    void GenerateSegment(float startX, int width)
    {
        SegmentType segmentType = ChooseSegmentType();
        
        switch (segmentType)
        {
            case SegmentType.Flat:
                GenerateFlatSegment(startX, width);
                break;
            case SegmentType.Gaps:
                GenerateGapSegment(startX, width);
                break;
            case SegmentType.Stairs:
                GenerateStairsSegment(startX, 7);
                break;
            case SegmentType.Platforms:
                GeneratePlatformsSegment(startX, width);
                break;
            case SegmentType.Coins:
                GenerateCoinsSegment(startX, width);
                break;
        }
    }

    SegmentType ChooseSegmentType()
    {
       // Randomly choose a segment type for now
        Array segmentTypes = Enum.GetValues(typeof(SegmentType));
        return (SegmentType)segmentTypes.GetValue(new Random().Next(segmentTypes.Length));
    }

    void GenerateFlatSegment(float startX, int width)
    {
        for (float x = startX; x < startX + width; x += groundTextureWidth)
        {
            SpawnTile(groundTexture, new Vector2(x, groundHeight));
        }
    }

    void GenerateGapSegment(float startX, int width)
    {
        for (float x = startX; x < startX + width; x += groundTextureWidth)
        {
            if (new Random().Next(0, 2) == 0) // Randomly create gaps
            {
                continue; // Skip this tile to create a gap
            }
            SpawnTile(groundTexture, new Vector2(x, groundHeight));
        }
    }

    void GenerateStairsSegment(float startX, int width)
    {
        int height = 0;
        float len = startX + width;
        for (float y = groundHeight; y <= height; y += groundTextureHeight)
        {
            for (float x = startX; x < len; x += groundTextureWidth)
            {
                SpawnTile(groundTexture, new Vector2(x, y));
            }
            startX += groundTextureWidth; // Move right for the next step
        }
    }

    void GeneratePlatformsSegment(float startX, int width)
    {
        GenerateFlatSegment(startX, width); // Base ground

        for (float x = startX + 1; x < startX + width - 1; x += 3)
        {
            // Create a platform above the ground
            SpawnTile(platformTexture, new Vector2(x, groundHeight + 3));
        }
    }

    void GenerateCoinsSegment(float startX, int width)
    {
       //TODO: Implement coin generation logic
    }

    void SpawnTile(Texture2D texture, Vector2 position)
    {
        Entity tile = new Entity("Tile");
        tile.AddComponent<TransformComponent>().Position = new Vector3(position, 0);
        tile.AddComponent<SpriteRendererComponent>().Texture = texture;
    }
}
