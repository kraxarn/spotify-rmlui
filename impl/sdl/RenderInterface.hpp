#pragma once

#include "RmlUi/Core.h"

#include <SDL.h>
#include <SDL_image.h>
#include <GL/glew.h>

// Based off
// https://github.com/mikke89/RmlUi/blob/master/Samples/basic/sdl2/src/RenderInterfaceSDL2.h

namespace Sdl
{
	class RenderInterface: public Rml::RenderInterface
	{
	public:
		RenderInterface(SDL_Renderer *renderer, SDL_Window *window)
			: sdl_renderer(renderer),
			sdl_window(window)
		{
		}

		/**
		 * Called by RmlUi when it wants to render geometry that it does not wish to optimise.
		 */
		void RenderGeometry(Rml::Vertex *vertices, int num_vertices, int *indices, int num_indices,
			Rml::TextureHandle texture, const Rml::Vector2f &translation) override
		{
			// SDL uses shaders that we need to disable here
			glUseProgramObjectARB(0);
			glPushMatrix();
			glTranslatef(translation.x, translation.y, 0);

			Rml::Vector<Rml::Vector2f> Positions(num_vertices);
			Rml::Vector<Rml::Colourb> Colors(num_vertices);
			Rml::Vector<Rml::Vector2f> TexCoords(num_vertices);
			float texw = 0.0f;
			float texh = 0.0f;

			SDL_Texture *sdl_texture = nullptr;
			if (texture != 0U)
			{
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				sdl_texture = (SDL_Texture *) texture;
				SDL_GL_BindTexture(sdl_texture, &texw, &texh);
			}

			for (int i = 0; i < num_vertices; i++)
			{
				Positions[i] = vertices[i].position;
				Colors[i] = vertices[i].colour;
				if (sdl_texture != nullptr)
				{
					TexCoords[i].x = vertices[i].tex_coord.x * texw;
					TexCoords[i].y = vertices[i].tex_coord.y * texh;
				}
				else
				{
					TexCoords[i] = vertices[i].tex_coord;
				}
			}

			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);
			glVertexPointer(2, GL_FLOAT, 0, &Positions[0]);
			glColorPointer(4, GL_UNSIGNED_BYTE, 0, &Colors[0]);
			glTexCoordPointer(2, GL_FLOAT, 0, &TexCoords[0]);

			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, indices);
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_COLOR_ARRAY);

			if (sdl_texture != nullptr)
			{
				SDL_GL_UnbindTexture(sdl_texture);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			}

			glColor4f(1.0, 1.0, 1.0, 1.0);
			glPopMatrix();
			/* Reset blending and draw a fake point just outside the screen to let SDL know that it needs to reset its state in case it wants to render a texture */
			glDisable(GL_BLEND);
			SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_NONE);
			SDL_RenderDrawPoint(sdl_renderer, -1, -1);
		}

		/**
		 * Called by RmlUi when it wants to enable or disable scissoring to clip content.
		 */
		void EnableScissorRegion(bool enable) override
		{
			if (enable)
			{
				glEnable(GL_SCISSOR_TEST);
			}
			else
			{
				glDisable(GL_SCISSOR_TEST);
			}
		}

		/**
		 * Called by RmlUi when it wants to change the scissor region.
		 */
		void SetScissorRegion(int x, int y, int width, int height) override
		{
			int w_width;
			int w_height;
			SDL_GetWindowSize(sdl_window, &w_width, &w_height);
			glScissor(x, w_height - (y + height), width, height);
		}

		/**
		 * Called by RmlUi when a texture is required by the library.
		 */
		auto LoadTexture(Rml::TextureHandle &texture_handle, Rml::Vector2i &texture_dimensions,
			const Rml::String &source) -> bool override
		{
			Rml::FileInterface *file_interface = Rml::GetFileInterface();
			Rml::FileHandle file_handle = file_interface->Open(source);
			if (file_handle == 0U)
			{
				return false;
			}

			file_interface->Seek(file_handle, 0, SEEK_END);
			auto buffer_size = file_interface->Tell(file_handle);
			file_interface->Seek(file_handle, 0, SEEK_SET);

			auto *buffer = new char[buffer_size];
			file_interface->Read(buffer, buffer_size, file_handle);
			file_interface->Close(file_handle);

			size_t i;
			for (i = source.length() - 1; i > 0; i--)
			{
				if (source[i] == '.')
				{
					break;
				}
			}

			Rml::String extension = source.substr(i + 1, source.length() - i);

			auto *surface = IMG_LoadTyped_RW(SDL_RWFromMem(buffer, int(buffer_size)),
				1, extension.c_str());

			if (surface != nullptr)
			{
				SDL_Texture *texture = SDL_CreateTextureFromSurface(sdl_renderer, surface);

				if (texture != nullptr)
				{
					texture_handle = (Rml::TextureHandle) texture;
					texture_dimensions = Rml::Vector2i(surface->w, surface->h);
					SDL_FreeSurface(surface);
				}
				else
				{
					return false;
				}

				return true;
			}

			return false;
		}

		/**
		 * Called by RmlUi when a texture is required to be built from an internally-generated
		 * sequence of pixels.
		 */
		auto GenerateTexture(Rml::TextureHandle &texture_handle, const Rml::byte *source,
			const Rml::Vector2i &source_dimensions) -> bool override
		{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			Uint32 rmask = 0xff000000;
			Uint32 gmask = 0x00ff0000;
			Uint32 bmask = 0x0000ff00;
			Uint32 amask = 0x000000ff;
#else
			Uint32 rmask = 0x000000ff;
			Uint32 gmask = 0x0000ff00;
			Uint32 bmask = 0x00ff0000;
			Uint32 amask = 0xff000000;
#endif

			SDL_Surface *surface = SDL_CreateRGBSurfaceFrom((void *) source, source_dimensions.x,
				source_dimensions.y, 32, source_dimensions.x * 4,
				rmask, gmask, bmask, amask);
			SDL_Texture *texture = SDL_CreateTextureFromSurface(sdl_renderer, surface);
			SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
			SDL_FreeSurface(surface);
			texture_handle = (Rml::TextureHandle) texture;

			return true;
		}

		/**
		 * Called by RmlUi when a loaded texture is no longer required.
		 */
		void ReleaseTexture(Rml::TextureHandle texture_handle) override
		{
			SDL_DestroyTexture((SDL_Texture *) texture_handle);
		}

	private:
		SDL_Renderer *sdl_renderer = nullptr;
		SDL_Window *sdl_window = nullptr;
	};
}