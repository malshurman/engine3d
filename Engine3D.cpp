/*
 * Documentation: https://github.com/OneLoneCoder/olcPixelGameEngine
 */

#define OLC_PGE_APPLICATION
#include "objects.h"
#include <fstream>
#include <strstream>
#include <algorithm>

class Engine3D : public olc::PixelGameEngine
{
public:
	Engine3D()
	{
		sAppName = "3D Demo";
	}

private:
	Mesh meshCube;
	Mat4x4 matProj;	// Matrix that converts from view space to screen space
	Vec3d vCamera;	// Location of camera in world space
	Vec3d vLookDir;	// Direction vector along the direction camera points
	float fYaw;		// FPS Camera rotation in XZ plane
	float fTheta;	// Spins World transform

public:
	bool OnUserCreate() override
	{
		// Load object file
		meshCube = loadTrianglesFromObj("mountains.obj");

		// Projection Matrix
		matProj = Matrix_MakeProjection(90.0f, (float)ScreenHeight() / (float)ScreenWidth(), 0.1f, 4000.0f);
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		if (GetKey(olc::UP).bHeld)
			vCamera.y += 8.0f * fElapsedTime;	// Travel Upwards

		if (GetKey(olc::DOWN).bHeld)
			vCamera.y -= 8.0f * fElapsedTime;	// Travel Downwards


		// Dont use these two in FPS mode, it is confusing :P
		if (GetKey(olc::LEFT).bHeld)
			vCamera.x -= 8.0f * fElapsedTime;	// Travel Along X-Axis

		if (GetKey(olc::RIGHT).bHeld)
			vCamera.x += 8.0f * fElapsedTime;	// Travel Along X-Axis
		///////


		Vec3d vForward = vLookDir * (8.0f * fElapsedTime);

		// Standard FPS Control scheme, but turn instead of strafe
		if (GetKey(olc::W).bHeld)
			vCamera = vCamera + vForward;

		if (GetKey(olc::S).bHeld)
			vCamera = vCamera - vForward;

		if (GetKey(olc::A).bHeld)
			fYaw -= 2.0f * fElapsedTime;

		if (GetKey(olc::D).bHeld)
			fYaw += 2.0f * fElapsedTime;

		// Set up "World Tranmsform" though not updating theta 
		// makes this a bit redundant
		Mat4x4 matRotZ, matRotX;
		//fTheta += 1.0f * fElapsedTime; // Uncomment to spin me right round baby right
		matRotZ = Matrix_MakeRotationZ(fTheta * 0.5f);
		matRotX = Matrix_MakeRotationX(fTheta);

		Mat4x4 matTrans;
		matTrans = Matrix_MakeTranslation(0.0f, 0.0f, 5.0f);

		Mat4x4 matWorld;
		matWorld = createMatrixIdentity();	// Form World Matrix
		matWorld = matRotZ * matRotX; // Transform by rotation
		matWorld = matWorld * matTrans; // Transform by translation

		// Create "Point At" Matrix for camera
		Vec3d vUp = { 0,1,0 };
		Vec3d vTarget = { 0,0,1 };
		Mat4x4 matCameraRot = Matrix_MakeRotationY(fYaw);
		vLookDir = matCameraRot * vTarget;
		vTarget = vCamera + vLookDir;
		Mat4x4 matCamera = Matrix_PointAt(vCamera, vTarget, vUp);

		// Make view matrix from camera
		Mat4x4 matView = Matrix_QuickInverse(matCamera);

		// Store triagles for rastering later
		std::vector<Triangle> vecTrianglesToRaster;

		// Draw Triangles
		for (auto tri : meshCube.tris)
		{
			Triangle triProjected, triTransformed, triViewed;

			// World Matrix Transform
			triTransformed.p[0] = matWorld * tri.p[0];
			triTransformed.p[1] = matWorld * tri.p[1];
			triTransformed.p[2] = matWorld * tri.p[2];

			// Calculate triangle Normal
			Vec3d normal, line1, line2;

			// Get lines either side of triangle
			line1 = triTransformed.p[1] - triTransformed.p[0];
			line2 = triTransformed.p[2] - triTransformed.p[0];

			// Take cross product of lines to get normal to triangle surface
			normal = line1 % line2;

			// You normally need to normalise a normal!
			normal = ~normal;

			// Get Ray from triangle to camera
			Vec3d vCameraRay = triTransformed.p[0] - vCamera;

			// If ray is aligned with normal, then triangle is visible
			if ((normal * vCameraRay) < 0.0f)
			{
				// Illumination
				Vec3d light_direction = { 0.0f, 1.0f, -1.0f };
				light_direction = ~light_direction;

				// How "aligned" are light direction and triangle surface normal?
				float dp = std::max(0.1f, (light_direction * normal));

				// Choose console colours as required (much easier with RGB)
				triTransformed.col = olc::Pixel(255 * dp, 255 * dp, 255 * dp).n;

				// Convert World Space --> View Space
				triViewed.p[0] = matView * triTransformed.p[0];
				triViewed.p[1] = matView * triTransformed.p[1];
				triViewed.p[2] = matView * triTransformed.p[2];
				triViewed.col = triTransformed.col;

				// Clip Viewed Triangle against near plane, this could form two additional
				// additional triangles. 
				int nClippedTriangles = 0;
				Triangle clipped[2];
				nClippedTriangles = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, triViewed, clipped[0], clipped[1]);

				// We may end up with multiple triangles form the clip, so project as
				// required
				for (int n = 0; n < nClippedTriangles; n++)
				{
					// Project triangles from 3D --> 2D
					triProjected.p[0] = matProj * clipped[n].p[0];
					triProjected.p[1] = matProj * clipped[n].p[1];
					triProjected.p[2] = matProj * clipped[n].p[2];
					triProjected.col = clipped[n].col;

					// Scale into view, we moved the normalising into cartesian space
					// out of the matrix.vector function from the previous videos, so
					// do this manually
					triProjected.p[0] = triProjected.p[0] / triProjected.p[0].w;
					triProjected.p[1] = triProjected.p[1] / triProjected.p[1].w;
					triProjected.p[2] = triProjected.p[2] / triProjected.p[2].w;

					// X/Y are inverted so put them back
					triProjected.p[0].x *= -1.0f;
					triProjected.p[1].x *= -1.0f;
					triProjected.p[2].x *= -1.0f;
					triProjected.p[0].y *= -1.0f;
					triProjected.p[1].y *= -1.0f;
					triProjected.p[2].y *= -1.0f;

					// Offset verts into visible normalised space
					Vec3d vOffsetView = { 1,1,0 };
					triProjected.p[0] = triProjected.p[0] + vOffsetView;
					triProjected.p[1] = triProjected.p[1] + vOffsetView;
					triProjected.p[2] = triProjected.p[2] + vOffsetView;
					triProjected.p[0].x *= 0.5f * (float)ScreenWidth();
					triProjected.p[0].y *= 0.5f * (float)ScreenHeight();
					triProjected.p[1].x *= 0.5f * (float)ScreenWidth();
					triProjected.p[1].y *= 0.5f * (float)ScreenHeight();
					triProjected.p[2].x *= 0.5f * (float)ScreenWidth();
					triProjected.p[2].y *= 0.5f * (float)ScreenHeight();

					// Store triangle for sorting
					vecTrianglesToRaster.push_back(triProjected);
				}
			}
		}

		// Sort triangles from back to front
		sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](Triangle& t1, Triangle& t2)
			{
				float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
				float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
				return z1 > z2;
			});

		// Clear Screen
		Clear(olc::BLACK);

		// Loop through all transformed, viewed, projected, and sorted triangles
		for (auto& triToRaster : vecTrianglesToRaster)
		{
			// Clip triangles against all four screen edges, this could yield
			// a bunch of triangles, so create a queue that we traverse to 
			//  ensure we only test new triangles generated against planes
			Triangle clipped[2];
			std::list<Triangle> listTriangles;

			// Add initial triangle
			listTriangles.push_back(triToRaster);
			int nNewTriangles = 1;

			for (int p = 0; p < 4; p++)
			{
				int nTrisToAdd = 0;
				while (nNewTriangles > 0)
				{
					// Take triangle from front of queue
					Triangle test = listTriangles.front();
					listTriangles.pop_front();
					nNewTriangles--;

					// Clip it against a plane. We only need to test each 
					// subsequent plane, against subsequent new triangles
					// as all triangles after a plane clip are guaranteed
					// to lie on the inside of the plane. I like how this
					// comment is almost completely and utterly justified
					switch (p)
					{
					case 0:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 1:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, (float)ScreenHeight() - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 2:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 3:	nTrisToAdd = Triangle_ClipAgainstPlane({ (float)ScreenWidth() - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					}

					// Clipping may yield a variable number of triangles, so
					// add these new ones to the back of the queue for subsequent
					// clipping against next planes
					for (int w = 0; w < nTrisToAdd; w++)
						listTriangles.push_back(clipped[w]);
				}
				nNewTriangles = listTriangles.size();
			}


			// Draw the transformed, viewed, clipped, projected, sorted, clipped triangles
			for (auto& t : listTriangles)
			{
				FillTriangle(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, olc::Pixel(t.col));
				//DrawTriangle(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, PIXEL_SOLID, FG_BLACK);
			}
		}

		return true;
	}
};


int main()
{
	Engine3D window;
	if (window.Construct(1024, 768, 1, 1))
		window.Start();
	return 0;
}

