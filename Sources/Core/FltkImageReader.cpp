//
//  FltkImageReader.cpp
//  OpenSpades
//
//  Created by yvt on 7/22/13.
//  Copyright (c) 2013 yvt.jp. All rights reserved.
//

//#include "FltkImageReader.h"
#include "IBitmapCodec.h"
#include "Debug.h"
#include "Exception.h"
#include "IStream.h"
#include "Bitmap.h"
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include <string.h>

namespace spades {
	class FltkImageReader: public IBitmapCodec {
	public:
		virtual std::string GetName() = 0;
		
		virtual Fl_RGB_Image *LoadFltkImage(const std::string& data) = 0;
		
		virtual bool CanLoad() { return true; }
		virtual bool CanSave() { return false; }
		
		virtual bool CheckExtension(const std::string&) = 0;
		
		virtual Bitmap *Load(IStream *stream){
			SPADES_MARK_FUNCTION();
			
			// read all
			std::string data = stream->ReadAllBytes();
			
			// copy to buffer
			Fl_Image *img = LoadFltkImage(data);
			
			SPAssert(img);
			SPAssert(img->count() >= 1);
			
			const unsigned char* inPixels =
			(const unsigned char *)img->data()[0];
			int depth = img->d();
			int width = img->w();
			int height = img->h();
			int pitch = width * depth + img->ld();
			
			Bitmap *bmp;
			try{
				bmp = new Bitmap(width, height);
			}catch(...){
				delete img;
				throw;
			}
			try{
				unsigned char *outPixels = (unsigned char *)bmp->GetPixels();
				
				if(pitch == width * 4 && depth == 4){
					// if the format matches the requirement of Bitmap,
					// just use it
					memcpy(outPixels, inPixels, pitch * height);
				} else {
					// convert
					const unsigned char* line;
					for(int y = 0; y < height; y++){
						line = inPixels;
						for(int x = 0; x < width; x++){
							uint8_t r, g, b, a;
							switch(depth){
								case 1:
									r = g = b = *(line++);
									a = 255;
									break;
								case 2:
									r = g = b = *(line++);
									a = *(line++);
									break;
								case 3:
									r = *(line++);
									g = *(line++);
									b = *(line++);
									a = 255;
									break;
								case 4:
									r = *(line++);
									g = *(line++);
									b = *(line++);
									a = *(line++);
									break;
								default:
									SPAssert(false);
							}
							*(outPixels++) = r;
							*(outPixels++) = g;
							*(outPixels++) = b;
							*(outPixels++) = a;
						}
						inPixels += pitch;
					}
				}
				delete img;
				return bmp;
			}catch(...){
				delete img;
				delete bmp;
				throw;
			}
		}
		
		virtual void Save(IStream *, Bitmap *) {
			SPADES_MARK_FUNCTION();
			SPNotImplemented();
		}
	};
	
	class FltkPngReader: public FltkImageReader {
	public:
		virtual std::string GetName(){
			return "FLTK PNG Reader";
		}
		
		virtual Fl_RGB_Image *LoadFltkImage(const std::string& data) {
			return new Fl_PNG_Image(NULL, (const unsigned char *)data.data(),
									data.size());
		}
		
		virtual bool CheckExtension(const std::string& fn) {
			return EndsWith(fn, ".png");
		}
	} pngReader;
	
	class FltkJpegReader: public FltkImageReader {
	public:
		virtual std::string GetName(){
			return "FLTK JPEG Reader";
		}
		
		virtual Fl_RGB_Image *LoadFltkImage(const std::string& data) {
			return new Fl_JPEG_Image(NULL, (const unsigned char *)data.data());
		}
		
		virtual bool CheckExtension(const std::string& fn) {
			return EndsWith(fn, ".jpg") ||
			EndsWith(fn, ".jpeg") || EndsWith(fn, ".jpe");
		}
	} jpegReader;
	
}

