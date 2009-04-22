/*
* Copyright (C) 2007-2008 Gilles Gigan (gilles.gigan@gmail.com)
* eResearch Centre, James Cook University (eresearch.jcu.edu.au)
*
* This program was developed as part of the ARCHER project
* (Australian Research Enabling Environment) funded by a
* Systemic Infrastructure Initiative (SII) grant and supported by the Australian
* Department of Innovation, Industry, Science and Research
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public  License as published by the
* Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/
#include <jni.h>
#include <stdint.h>

#include "libvideo.h"
#include "palettes.h"
#include "common.h"
#include "debug.h"

static int add_format(JNIEnv *e, jobject vector, jmethodID add_method,
		jclass format_class,jmethodID format_ctor, int index,
		struct v4l4j_device *d){
	jobject obj;
	obj = (*e)->NewObject(e,
			format_class,
			format_ctor,
			(*e)->NewStringUTF(e,
					(const char *)libvideo_palettes[index].name
					)
			, index,
			(uintptr_t) d
			);
	if(obj==NULL){
		info("[V4L4J] Error creating the ImageFormat object\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error creating the ImageFormat object");
		return -1;
	}
	(*e)->CallVoidMethod(e, vector, add_method, obj);
	return 0;
}

static int create_JPEG_list(JNIEnv *e, struct v4l4j_device *d,
		jobject formats, jmethodID add_method, jclass format_class,
		jmethodID format_ctor ){
	int i, j;
	struct device_info *di = d->vdev->info;
	int jpeg_conv_formats[] = JPEG_CONVERTIBLE_FORMATS;

	dprint(LOG_V4L4J, "[V4L4J] Creating JPEG encodable format list\n");

	/* Populates the JPEGformats list*/

	for(i=0; i<di->nb_palettes; i++){
		if(di->palettes[i].index==JPEG && !di->palettes[i].raw_palettes){
			//if there is native JPEG format, add it
			dprint(LOG_V4L4J,
					"[V4L4J] Found native JPEG format - adding it to list\n");

			if(add_format(e,formats,add_method,format_class,
					format_ctor, JPEG, d)==-1)
				return -1;

		} else {
			//check if v4l4j can convert it
			for(j=0; j<ARRAY_SIZE(jpeg_conv_formats); j++){

				//if v4l4j knows how to convert it
				if(di->palettes[i].index==jpeg_conv_formats[j]){

					dprint(LOG_V4L4J, "[V4L4J] Found v4l4j-convertible JPEG "
							"format from %s format - add it\n",
							libvideo_palettes[jpeg_conv_formats[j]].name);

					if(add_format(e,formats,add_method,format_class,
							format_ctor, jpeg_conv_formats[j], d)==-1)
						return -1;
				}
			} //end for v4l4j JPEG convertible formats
		}
	}//end for all supported formats

	return 0;
}

static int create_RGB_list(JNIEnv *e, struct v4l4j_device *d,
		jobject formats, jmethodID add_method, jclass format_class,
		jmethodID format_ctor ){
	int i, j;
	struct device_info *di = d->vdev->info;
	int v4l4j_rgb24_conv_formats[] = RGB24_CONVERTIBLE_FORMATS;

	dprint(LOG_V4L4J, "[V4L4J] Creating RGB encodable format list\n");

	/* Populates the RGBformats list*/

	for(i=0; i<di->nb_palettes; i++){
		if(di->palettes[i].index==RGB24 && !di->palettes[i].raw_palettes){
			//if there is native RGB24 format, add it
			dprint(LOG_V4L4J,
					"[V4L4J] Found native RGB24 format - adding it to list\n");

			if(add_format(e,formats,add_method,format_class,
					format_ctor, RGB24, d)==-1)
				return -1;

		} else if(di->palettes[i].index==RGB24 && di->palettes[i].raw_palettes){
			//if there are libvideo-converted formats, add them
			j=-1;
			while(di->palettes[i].raw_palettes[++j]!=-1){
				dprint(LOG_V4L4J, "[V4L4J] Found libvideo-converted RGB24 "
						"format from %s format - add it\n",
						libvideo_palettes[di->palettes[i].raw_palettes[j]].name);

				if(add_format(e,formats,add_method,format_class,
						format_ctor, di->palettes[i].raw_palettes[j], d)==-1)
					return -1;
			}

		} else {
			//check if v4l4j can convert it
			for(j=0; j<ARRAY_SIZE(v4l4j_rgb24_conv_formats); j++){

				//if it is a native format AND v4l4j knows how to convert it
				if(!di->palettes[i].raw_palettes &&
						di->palettes[i].index==v4l4j_rgb24_conv_formats[j]){

					dprint(LOG_V4L4J, "[V4L4J] Found v4l4j provided RGB24 "
							"format from %s format - add it\n",
							libvideo_palettes[v4l4j_rgb24_conv_formats[j]].name);

					if(add_format(e,formats,add_method,format_class,
							format_ctor, v4l4j_rgb24_conv_formats[j], d)==-1)
						return -1;
				}
			} //end for v4l4j rgv24 convertible formats
		}
	}//end for all supported formats

	return 0;
}

static int create_BGR_list(JNIEnv *e, struct v4l4j_device *d,
		jobject formats, jmethodID add_method, jclass format_class,
		jmethodID format_ctor ){
	int i, j;
	struct device_info *di = d->vdev->info;

	dprint(LOG_V4L4J, "[V4L4J] Creating BGR24 encodable format list\n");

	/* Populates the BGRformats list*/

	for(i=0; i<di->nb_palettes; i++){
		if(di->palettes[i].index==BGR24 && !di->palettes[i].raw_palettes){
			//if there is native BGR24 format, add it
			dprint(LOG_V4L4J,
					"[V4L4J] Found native BGR24 format - adding it to list\n");

			if(add_format(e,formats,add_method,format_class,
					format_ctor, BGR24, d)==-1)
				return -1;

		} else if(di->palettes[i].index==BGR24 && di->palettes[i].raw_palettes){
			//if there are libvideo-converted BGR24 formats, add them
			j=-1;
			while(di->palettes[i].raw_palettes[++j]!=-1){
				dprint(LOG_V4L4J, "[V4L4J] Found libvideo-converted BGR24 "
						"format from %s format - add it\n",
						libvideo_palettes[di->palettes[i].raw_palettes[j]].name);

				if(add_format(e,formats,add_method,format_class,
						format_ctor, di->palettes[i].raw_palettes[j], d)==-1)
					return -1;
			}
		}
	}

	return 0;
}

static int create_YUV_list(JNIEnv *e, struct v4l4j_device *d,
		jobject formats, jmethodID add_method, jclass format_class,
		jmethodID format_ctor ){
	int i,j;
	struct device_info *di = d->vdev->info;

	dprint(LOG_V4L4J, "[V4L4J] Creating YUV encodable format list\n");

	/* Populates the YUVformats list*/

	for(i=0; i<di->nb_palettes; i++){
		if(di->palettes[i].index==YUV420 && !di->palettes[i].raw_palettes){
			//if there is native YUV420 format, add it
			dprint(LOG_V4L4J,
					"[V4L4J] Found native YUV420 format - adding it to list\n");

			if(add_format(e,formats,add_method,format_class, format_ctor,
					YUV420, d)==-1)
				return -1;

		} else if(di->palettes[i].index==YUV420 && di->palettes[i].raw_palettes){
			//if there are libvideo-converted YUV420 format, add them
			j=-1;
			while(di->palettes[i].raw_palettes[++j]!=-1){
				dprint(LOG_V4L4J, "[V4L4J] Found libvideo-converted YUV420 "
						"format from %s format - add it\n",
						libvideo_palettes[di->palettes[i].raw_palettes[j]].name);

				if(add_format(e,formats,add_method,format_class,
						format_ctor, di->palettes[i].raw_palettes[j], d)==-1)
					return -1;
			}
		}
	}

	return 0;
}

static int create_YVU_list(JNIEnv *e, struct v4l4j_device *d,
		jobject formats, jmethodID add_method, jclass format_class,
		jmethodID format_ctor ){
	int i,j;
	struct device_info *di = d->vdev->info;

	dprint(LOG_V4L4J, "[V4L4J] Creating YVU encodable format list\n");

	/* Populates the YUVformats list*/

	for(i=0; i<di->nb_palettes; i++){
		if(di->palettes[i].index==YVU420 && !di->palettes[i].raw_palettes){
			//if there is native YVU420 format, add it
			dprint(LOG_V4L4J,
					"[V4L4J] Found native YVU420 format - adding it to list\n");

			if(add_format(e,formats,add_method,format_class, format_ctor,
					YVU420,d )==-1)
				return -1;

		} else if(di->palettes[i].index==YVU420 && di->palettes[i].raw_palettes){
			//if there are libvideo-converted YVU420 format, add them
			j=-1;
			while(di->palettes[i].raw_palettes[++j]!=-1){
				dprint(LOG_V4L4J, "[V4L4J] Found libvideo-converted YVU420 "
						"format from %s format - add it\n",
						libvideo_palettes[di->palettes[i].raw_palettes[j]].name);

				if(add_format(e,formats,add_method,format_class,
						format_ctor, di->palettes[i].raw_palettes[j],d)==-1)
					return -1;
			}
		}
	}

	return 0;
}

static int create_native_list(JNIEnv *e, struct v4l4j_device *d,
		jobject formats, jmethodID add_method, jclass format_class,
		jmethodID format_ctor ){
	int i;
	struct device_info *di = d->vdev->info;

	dprint(LOG_V4L4J, "[V4L4J] Creating native format list\n");

	/* Populates the native formats list*/
	for(i=0; i<di->nb_palettes; i++){
		dprint(LOG_V4L4J, "[V4L4J] Checking format %s - index: %d - raw ? %s\n",
				libvideo_palettes[di->palettes[i].index].name,
				di->palettes[i].index,
				(!di->palettes[i].raw_palette?"Yes-adding it":"No-skipping it")
				);

		if(!di->palettes[i].raw_palettes)
			//it is a native format, add it to the list
			if(add_format(e, formats,add_method, format_class,
					format_ctor, di->palettes[i].index,d)==-1)
				return -1;
	}

	return 0;
}

/*
 * populate the formats, JPEGformats, RGBformats, YUVformats & YVUformats
 * members of the ImageFormat class with appropriate image formats
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_ImageFormatList_listFormats(
		JNIEnv *e, jobject t, jlong v4l4j_device){
	jclass vector_class, format_class, this_class;
	jmethodID add_method, format_ctor;
	jfieldID field;
	jobject obj;
	struct v4l4j_device *d = (struct v4l4j_device *) (uintptr_t) v4l4j_device;

	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	/* Get handles on Java stuff */
	this_class = (*e)->GetObjectClass(e, t);
	if(this_class == NULL){
		info("[V4L4J] Error looking up the ImageFormatList class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up ImageFormatList class");
		return;
	}

	format_class = (*e)->FindClass(e, "au/edu/jcu/v4l4j/ImageFormat");
	if(format_class == NULL){
		info("[V4L4J] Error looking up the ImageFormat class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up ImageFormat class");
		return;
	}

	format_ctor = (*e)->GetMethodID(e, format_class, "<init>",
			"(Ljava/lang/String;IJ)V");
	if(format_ctor == NULL){
		info("[V4L4J] Error looking up the constructor of ImageFormat class\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error looking up the constructor of ImageFormat class");
		return;
	}

	vector_class = (*e)->FindClass(e, "java/util/Vector");
	if(vector_class == NULL){
		info("[V4L4J] Error looking up the Vector class\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up Vector class");
		return;
	}

	add_method = (*e)->GetMethodID(e, vector_class, "addElement",
			"(Ljava/lang/Object;)V");
	if(add_method == NULL){
		info("[V4L4J] Error looking up the add method of Vector class\n");
		THROW_EXCEPTION(e, JNI_EXCP, \
				"Error looking up the add method of Vector class");
		return;
	}

	//
	// populates the native formats in the formats field
	//
	field = (*e)->GetFieldID(e, this_class, "formats", "Ljava/util/List;");
	if(field == NULL){
		info("[V4L4J] Error looking up the formats fieldID\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the formats fieldID");
		return;
	}

	obj = (*e)->GetObjectField(e, t, field);
	if(field == NULL){
		info("[V4L4J] Error looking up the formats member\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the formats member");
		return;
	}

	/* creates the list object and assign it to the formats attribute*/
	if(create_native_list(e,d, obj, add_method, format_class,format_ctor)==-1)
		return;



	//
	// populates the RGBformats field
	//
	field = (*e)->GetFieldID(e, this_class, "RGBformats", "Ljava/util/List;");
	if(field == NULL){
		info("[V4L4J] Error looking up the RGBformats fieldID\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the RGBformats fieldID");
		return;
	}

	obj = (*e)->GetObjectField(e, t, field);
	if(field == NULL){
		info("[V4L4J] Error looking up the RGBformats member\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the RGBformats member");
		return;
	}

	/* creates the list object and assign it to the RGBformats attribute*/
	if(create_RGB_list(e,d, obj, add_method, format_class,format_ctor)==-1)
		return;




	//
	// populates the BGRformats field
	//
	field = (*e)->GetFieldID(e, this_class, "BGRformats", "Ljava/util/List;");
	if(field == NULL){
		info("[V4L4J] Error looking up the BGRformats fieldID\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the BGRformats fieldID");
		return;
	}

	obj = (*e)->GetObjectField(e, t, field);
	if(field == NULL){
		info("[V4L4J] Error looking up the BGRformats member\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the BGRformats member");
		return;
	}

	/* creates the list object and assign it to the BGRformats attribute*/
	if(create_BGR_list(e,d, obj, add_method, format_class,format_ctor)==-1)
		return;





	//
	// populates the YUV420formats field
	//
	field = (*e)->GetFieldID(e, this_class, "YUV420formats", "Ljava/util/List;");
	if(field == NULL){
		info("[V4L4J] Error looking up the YUV420formats fieldID\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the YUV420formats fieldID");
		return;
	}

	obj = (*e)->GetObjectField(e, t, field);
	if(field == NULL){
		info("[V4L4J] Error looking up the YUV420formats member\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the YUV420formats member");
		return;
	}

	/* creates the list object and assign it to the YUV420formats attribute*/
	if(create_YUV_list(e,d, obj, add_method, format_class,format_ctor)==-1)
		return;



	//
	// populates the YVU420formats field
	//
	field = (*e)->GetFieldID(e, this_class, "YVU420formats", "Ljava/util/List;");
	if(field == NULL){
		info("[V4L4J] Error looking up the YVU420formats fieldID\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the YVU420formats fieldID");
		return;
	}

	obj = (*e)->GetObjectField(e, t, field);
	if(field == NULL){
		info("[V4L4J] Error looking up the YVU420formats member\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the YVU420formats member");
		return;
	}

	/* creates the list object and assign it to the YVU420formats attribute*/
	if(create_YVU_list(e,d, obj, add_method, format_class,format_ctor)==-1)
		return;



	//
	// populates the JPEGformats field
	//
	field = (*e)->GetFieldID(e, this_class, "JPEGformats", "Ljava/util/List;");
	if(field == NULL){
		info("[V4L4J] Error looking up the JPEGformats fieldID\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the JPEGformats fieldID");
		return;
	}

	obj = (*e)->GetObjectField(e, t, field);
	if(field == NULL){
		info("[V4L4J] Error looking up the JPEGformats member\n");
		THROW_EXCEPTION(e, JNI_EXCP, "Error looking up the JPEGformats member");
		return;
	}

	/* creates the list object and assign it to the JPEGformats attribute*/
	if(create_JPEG_list(e,d, obj, add_method, format_class,format_ctor)==-1)
		return;
}

