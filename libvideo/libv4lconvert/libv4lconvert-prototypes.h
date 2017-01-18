/**
 * Stuff to deal with ImageTransformerPrototype's (registration, deregistration, lookup)
 * @author mailmindlin January 2017
 */
#ifndef __LIBV4LCONVERT_PROTOTYPES_H
#define __LIBV4LCONVERT_PROTOTYPES_H

#include "libv4lconvert-flat.h"
#include <collections/iterator/iterator.h>

/**
 * Registers an ImageTransformerPrototype. The caller MUST NOT free the prototype or alter it in any way
 * while it is registerred.
 * @return prototype id, or -1u on failure
 * May set errno on failure
 */
LIBV4LCONVERT_PUBLIC unsigned int ImageTransformerPrototype_register(ImageTransformerPrototype* prototype) __attribute__ ((nonnull (1)));
/**
 * Query for all ImageTransformerPrototype's that match the given query.
 * @param type
 *       Conversion type of result to search for.
 *       If this parameter is v4lconvert_conversion_type_unknown, it will act as a wildcard.
 *       If this parameter is not a valid conversion type, errno will be set to EINVAL and NULL will be returned.
 * @param sourcePalette
 *       Source libvideo palette id of prototypes to query.
 *       If this parameter is negative, it will act as a wildcard.
 *       If this parameter is positive but does not refer to any valid palette id, errno will be set to EINVAL and NULL will be returned.
 * @param outputPalette
 *       Output libvideo palette id of prototypes to query.
 *       If this parameter is negative, it will act as a wildcard.
 *       If this parameter is positive but does not refer to any valid palette id, errno will be set to EINVAL and NULL will be returned.
 * @return Iterator of ImageTransformerPrototype's found.
 *       Iterator->release should be called on the result after finished using.
 *       Will be NULL on error, upon which errno SHOULD be set.
 *       The iterator's behavior is undefined if, after calling this method, more ImageTransformerPrototype's are registered or deregistered
 */
LIBV4LCONVERT_PUBLIC Iterator*    ImageTransformerPrototype_query(v4lconvert_conversion_type type, int sourcePalette, int outputPalette);
/**
 * Look up an ImageTransformerPrototype by its id.
 * @return Reference to ImageTransformerPrototype.
 *       The returned reference may not be authorative, and SHOULD NOT be modified.
 *       Will return NULL if no ImageTransformerPrototype exists with the given ID, but SHOULD NOT set errno.
 */
LIBV4LCONVERT_PUBLIC ImageTransformerPrototype* ImageTransformerPrototype_lookupById(unsigned int id);
/**
 * Deregisters an ImageTransformerPrototype.
 * @return ImageTransformerPrototype pointer, which should be released by the user.
 *       Will return NULL if no ImageTransformerPrototype exists with the given ID, but SHOULD NOT set errno.
 */
LIBV4LCONVERT_PUBLIC ImageTransformerPrototype* ImageTransformerPrototype_deregister(unsigned int id);

#endif //__LIBV4LCONVERT_PROTOTYPES_H