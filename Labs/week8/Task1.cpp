// Enable the M_PI constant from <math.h> on MSVC-compatible toolchains.
#define _USE_MATH_DEFINES
#include <math.h>

#include <iostream>
#include <lodepng.h>
#include "Image.hpp"
#include "LinAlg.hpp"
#include "Light.hpp"
#include "Mesh.hpp"
#include "Shading.hpp"

// ***** WEEK 8 LAB *****
// This file implements Task 1 of the rasterisation lab.
//
// Goal:
// Render a mesh using the Phong reflection model, including ambient,
// diffuse, and specular lighting components.
//
// Task breakdown:
// 1. Implement the reflection helper in Shading.hpp.
// 2. Implement the Phong specular term in Shading.hpp.
// 3. Supply the correct lighting and view vectors during rasterisation.
// 4. Experiment with different light setups and shininess values.

struct Triangle {
	std::array<Eigen::Vector3f, 3> screen; // Vertex positions in screen space (x, y, depth).
	std::array<Eigen::Vector3f, 3> verts;  // Vertex positions in world space.
	std::array<Eigen::Vector3f, 3> norms;  // Per-vertex surface normals in world space.
	std::array<Eigen::Vector2f, 3> texs;   // Per-vertex texture coordinates.
};


Eigen::Matrix4f projectionMatrix(int height, int width, float horzFov = 70.f * M_PI / 180.f, float zFar = 10.f, float zNear = 0.1f)
{
	// Construct a simple perspective projection matrix from the horizontal
	// field of view and image aspect ratio.
	float vertFov = horzFov * float(height) / width;
	Eigen::Matrix4f projection;
	projection <<
		1.0f / tanf(0.5f * horzFov), 0, 0, 0,
		0, 1.0f / tanf(0.5f * vertFov), 0, 0,
		0, 0, zFar / (zFar - zNear), -zFar * zNear / (zFar - zNear),
		0, 0, 1, 0;
	return projection;
}

void findScreenBoundingBox(const Triangle& t, int width, int height, int& minX, int& minY, int& maxX, int& maxY)
{
	// Compute the axis-aligned bounding box of the triangle in screen space.
	// Restricting rasterisation to this region avoids unnecessary pixel tests.
	minX = std::min(std::min(t.screen[0].x(), t.screen[1].x()), t.screen[2].x());
	minY = std::min(std::min(t.screen[0].y(), t.screen[1].y()), t.screen[2].y());
	maxX = std::max(std::max(t.screen[0].x(), t.screen[1].x()), t.screen[2].x());
	maxY = std::max(std::max(t.screen[0].y(), t.screen[1].y()), t.screen[2].y());

	// Clamp the bounding box to the valid image region.
	minX = std::min(std::max(minX, 0), width - 1);
	maxX = std::min(std::max(maxX, 0), width - 1);
	minY = std::min(std::max(minY, 0), height - 1);
	maxY = std::min(std::max(maxY, 0), height - 1);
}


void drawTriangle(std::vector<uint8_t>& image, int width, int height,
	std::vector<float>& zBuffer,
	const Triangle& t,
	const std::vector<std::unique_ptr<Light>>& lights,
	const Eigen::Vector3f& albedo, const Eigen::Vector3f& specularColor,
	float specularExponent,
	const Eigen::Vector3f& camWorldPos)
{
	int minX, minY, maxX, maxY;
	findScreenBoundingBox(t, width, height, minX, minY, maxX, maxY);

	Eigen::Vector2f edge1 = v2(t.screen[2] - t.screen[0]);
	Eigen::Vector2f edge2 = v2(t.screen[1] - t.screen[0]);
	float triangleArea = 0.5f * vec2Cross(edge2, edge1);
	if (triangleArea < 0) {
		// Back-face culling: skip triangles whose winding indicates they face
		// away from the camera in screen space.
		return;
	}

	for (int x = minX; x <= maxX; ++x)
		for (int y = minY; y <= maxY; ++y) {
			Eigen::Vector2f p(x, y);

			// Compute the areas of the three sub-triangles formed with the pixel
			// sample point. These are used to derive barycentric coordinates.
			float a0 = 0.5f * fabsf(vec2Cross(v2(t.screen[1]) - v2(t.screen[2]), p - v2(t.screen[2])));
			float a1 = 0.5f * fabsf(vec2Cross(v2(t.screen[0]) - v2(t.screen[2]), p - v2(t.screen[2])));
			float a2 = 0.5f * fabsf(vec2Cross(v2(t.screen[0]) - v2(t.screen[1]), p - v2(t.screen[1])));

			// Convert sub-triangle areas into barycentric coordinates.
			float b0 = a0 / triangleArea;
			float b1 = a1 / triangleArea;
			float b2 = a2 / triangleArea;

			// If the barycentric coordinates do not sum to approximately 1,
			// the point lies outside the triangle.
			float sum = b0 + b1 + b2;
			if (sum > 1.0001) {
				continue;
			}

			// Interpolate the fragment position in world space.
			Eigen::Vector3f worldP = t.verts[0] * b0 + t.verts[1] * b1 + t.verts[2] * b2;

			// Interpolate depth and perform a z-buffer visibility test.
			float depth = t.screen[0].z() * b0 + t.screen[1].z() * b1 + t.screen[2].z() * b2;
			int depthIdx = static_cast<int>(p.x()) + static_cast<int>(p.y()) * width;
			if (depth > zBuffer[depthIdx]) continue;
			zBuffer[depthIdx] = depth;

			// Interpolate and normalise the surface normal for shading.
			Eigen::Vector3f normP = t.norms[0] * b0 + t.norms[1] * b1 + t.norms[2] * b2;
			normP.normalize();

			// Accumulate illumination from all light sources at this fragment.
			Eigen::Vector3f color = Eigen::Vector3f::Zero();

			for (auto& light : lights) {

				// Evaluate the light intensity reaching the current point.
				Eigen::Vector3f lightIntensity = light->getIntensityAt(worldP);

				if (light->getType() != Light::Type::AMBIENT) {

					// Compute the incoming light direction. In this codebase,
					// getDirection(worldP) returns a vector pointing from the light
					// toward the shaded point.
					Eigen::Vector3f incomingLightDir = light->getDirection(worldP);

					// Compute the viewing direction from the shaded point toward
					// the camera position.
					Eigen::Vector3f viewDir = (camWorldPos - worldP).normalized();

					// Evaluate the Phong specular term using the incoming light
					// direction, surface normal, view direction, and shininess.
					float specularTerm = phongSpecularTerm(incomingLightDir, normP, viewDir, specularExponent);

					Eigen::Vector3f specularOut = specularColor * specularTerm;
					specularOut = coeffWiseMultiply(specularOut, lightIntensity);

					// Lambertian diffuse response uses the cosine of the angle
					// between the normal and the outgoing light direction.
					float dotProd = normP.dot(-incomingLightDir);

					// Clamp negative values so that light contributes only when it
					// reaches the front-facing side of the surface.
					dotProd = std::max(dotProd, 0.0f);

					Eigen::Vector3f diffuseOut = lightIntensity * dotProd;
					diffuseOut = coeffWiseMultiply(diffuseOut, albedo);

					// Combine diffuse and specular contributions from this light.
					color += specularOut;
					color += diffuseOut;
				}
				else {
					// Ambient light is applied uniformly and is modulated only by
					// the surface albedo.
					color += coeffWiseMultiply(lightIntensity, albedo);
				}
			}

			Color c;
			// Apply gamma correction before converting to 8-bit colour values.
			c.r = std::min(powf(color.x(), 1 / 2.2f), 1.0f) * 255;
			c.g = std::min(powf(color.y(), 1 / 2.2f), 1.0f) * 255;
			c.b = std::min(powf(color.z(), 1 / 2.2f), 1.0f) * 255;

			c.a = 255;

			setPixel(image, x, y, width, height, c);
		}
}



void drawMesh(std::vector<unsigned char>& image,
	std::vector<float>& zBuffer,
	const Mesh& mesh,
	const Eigen::Vector3f& albedo, const Eigen::Vector3f& specularColor,
	float specularExponent,
	const Eigen::Vector3f& camWorldPos,
	const Eigen::Matrix4f& modelToWorld,
	const Eigen::Matrix4f& worldToClip,
	const std::vector<std::unique_ptr<Light>>& lights,
	int width, int height)
{
	for (int i = 0; i < mesh.vFaces.size(); ++i) {
		Eigen::Vector3f
			v0 = mesh.verts[mesh.vFaces[i][0]],
			v1 = mesh.verts[mesh.vFaces[i][1]],
			v2 = mesh.verts[mesh.vFaces[i][2]];
		Eigen::Vector3f
			n0 = mesh.norms[mesh.nFaces[i][0]],
			n1 = mesh.norms[mesh.nFaces[i][1]],
			n2 = mesh.norms[mesh.nFaces[i][2]];

		Triangle t;
		t.verts[0] = (modelToWorld * vec3ToVec4(v0)).block<3, 1>(0, 0);
		t.verts[1] = (modelToWorld * vec3ToVec4(v1)).block<3, 1>(0, 0);
		t.verts[2] = (modelToWorld * vec3ToVec4(v2)).block<3, 1>(0, 0);

		// Transform vertices into clip space and perform perspective divide
		// to obtain normalised device coordinates.
		Eigen::Vector4f vClip0 = worldToClip * modelToWorld * vec3ToVec4(v0);
		vClip0 /= vClip0.w();
		Eigen::Vector4f vClip1 = worldToClip * modelToWorld * vec3ToVec4(v1);
		vClip1 /= vClip1.w();
		Eigen::Vector4f vClip2 = worldToClip * modelToWorld * vec3ToVec4(v2);
		vClip2 /= vClip2.w();

		// Skip triangles with any vertex outside the clip volume.
		if (outsideClipBox(vClip0) || outsideClipBox(vClip1) || outsideClipBox(vClip2)) continue;

		// Convert from clip-space coordinates in [-1, 1] to image-space pixel coordinates.
		t.screen[0] = Eigen::Vector3f((vClip0.x() + 1.0f) * width / 2, (-vClip0.y() + 1.0f) * height / 2, vClip0.z());
		t.screen[1] = Eigen::Vector3f((vClip1.x() + 1.0f) * width / 2, (-vClip1.y() + 1.0f) * height / 2, vClip1.z());
		t.screen[2] = Eigen::Vector3f((vClip2.x() + 1.0f) * width / 2, (-vClip2.y() + 1.0f) * height / 2, vClip2.z());

		// Transform normals into world space using the inverse-transpose of
		// the linear part of the model transform.
		t.norms[0] = (modelToWorld.block<3, 3>(0, 0).inverse().transpose() * n0).normalized();
		t.norms[1] = (modelToWorld.block<3, 3>(0, 0).inverse().transpose() * n1).normalized();
		t.norms[2] = (modelToWorld.block<3, 3>(0, 0).inverse().transpose() * n2).normalized();

		t.texs[0] = mesh.texs[mesh.tFaces[i][0]];
		t.texs[1] = mesh.texs[mesh.tFaces[i][1]];
		t.texs[2] = mesh.texs[mesh.tFaces[i][2]];

		drawTriangle(image, width, height, zBuffer, t, lights, albedo, specularColor, specularExponent, camWorldPos);
	}
}


int main()
{
	std::string outputFilename = "output.png";

	const int width = 512, height = 512;
	const int nChannels = 4;

	// Allocate the colour buffer and depth buffer.
	// The image stores RGBA values with 8 bits per channel.
	std::vector<uint8_t> imageBuffer(height * width * nChannels);
	std::vector<float> zBuffer(height * width);

	// Initialise the image to black and the z-buffer to the farthest depth.
	Color black{ 0,0,0,255 };
	for (int r = 0; r < height; ++r) {
		for (int c = 0; c < width; ++c) {
			setPixel(imageBuffer, c, r, width, height, black);
			zBuffer[r * width + c] = 1.0f;
		}
	}

	Eigen::Matrix4f projection = projectionMatrix(height, width);

	// Position the camera slightly above the scene and tilt it downward.
	Eigen::Matrix4f cameraToWorld = translationMatrix(Eigen::Vector3f(0.f, 0.8f, 0.f)) * rotateXMatrix(0.4f);

	// Extract the camera position in world space.
	Eigen::Vector3f camWorldPos = (cameraToWorld * Eigen::Vector4f(0, 0, 0, 1)).block<3, 1>(0, 0);

	// Build the view and clip transforms.
	// worldToCamera is the inverse of cameraToWorld.
	Eigen::Matrix4f worldToCamera = cameraToWorld.inverse();

	// worldToClip combines the view transform with the perspective projection.
	Eigen::Matrix4f worldToClip = projection * worldToCamera;

	std::string bunnyFilename = "../models/stanford_bunny_texmapped.obj";
	std::string planeFilename = "../models/plane.obj";

	// Lighting configuration for the scene.
	std::vector<std::unique_ptr<Light>> lights;
	lights.emplace_back(new AmbientLight(Eigen::Vector3f(0.1f, 0.1f, 0.1f)));
	lights.emplace_back(new DirectionalLight(Eigen::Vector3f(0.4f, 0.4f, 0.4f), Eigen::Vector3f(1.f, -1.f, 0.0f)));

	Mesh bunnyMesh = loadMeshFile(bunnyFilename);
	Mesh planeMesh = loadMeshFile(planeFilename);

	Eigen::Matrix4f bunnyTransform;
	bunnyTransform = translationMatrix(Eigen::Vector3f(0.0f, -1.0f, 3.f)) * rotateYMatrix(M_PI);

	// Render the bunny with Phong shading.
	drawMesh(imageBuffer, zBuffer, bunnyMesh, Eigen::Vector3f(0.f, 0.5f, 0.8f),
		Eigen::Vector3f::Ones() * 1.0f, 10.f, camWorldPos,
		bunnyTransform, worldToClip, lights, width, height);

	Eigen::Matrix4f planeTransform;
	planeTransform = translationMatrix(Eigen::Vector3f(0.0f, -1.0f, 3.f)) * scaleMatrix(1.4f);

	// Render the ground plane.
	drawMesh(imageBuffer, zBuffer, planeMesh, Eigen::Vector3f(0.f, 0.5f, 0.8f),
		Eigen::Vector3f::Ones() * 1.0f, 10.f, camWorldPos,
		planeTransform, worldToClip, lights, width, height);

	// Draw markers for point lights to assist with debugging.
	drawPointLights(imageBuffer, width, height, lights);

	// Write the final colour buffer to disk.
	int errorCode;
	errorCode = lodepng::encode(outputFilename, imageBuffer, width, height);
	if (errorCode) {
		std::cout << "lodepng error encoding image: " << lodepng_error_text(errorCode) << std::endl;
		return errorCode;
	}

	// Save a visualisation of the z-buffer for inspection.
	saveZBufferImage("zBuffer.png", zBuffer, width, height);

	return 0;
}