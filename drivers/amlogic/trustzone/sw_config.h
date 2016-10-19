/*
 * Copyright (c) 2010-2013 Sierraware, LLC.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions and derivatives of the Software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification is strictly prohibited without prior written consent from
 * Sierraware, LLC.
 *
 * Redistribution in binary form must reproduce the above copyright  notice, 
 * this list of conditions and  the following disclaimer in the documentation 
 * and/or other materials  provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */
/* 
 * config variable declaration
 */

#ifndef SW_CONFIG
#define SW_CONFIG
#if defined(__GNUC__) && \
	defined(__GNUC_MINOR__) && \
defined(__GNUC_PATCHLEVEL__) && \
((__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)) \
>= 40700
#define USE_ARCH_EXTENSION_SEC 1
#else
#define USE_ARCH_EXTENSION_SEC 0
#endif
#endif
