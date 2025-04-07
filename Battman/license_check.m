/* Q: Why a non-paid software performs a license check?
 * A: We will start to use a "non-free" license in future,
 *    this is for ensure you have actually confirmed and
 *    agreeed the Term of Use before you start using it.
 * Q: Isn't it enough with a single mark like "Agreed=1"?
 * A: No, let's imagine the scene if somebody is
 *    redistributing the Battman data for sharing the
 *    app configurations or something else, the simplest
 *    boolean marks will eventually skip the license
 *    popup. What will happen if a person did not
 *    actually accepted the ToU before using Battman? */

#include "common.h"
#include "intlextern.h"
#import <Foundation/Foundation.h>
/* Happy CommonCrypto! What? You want OpenSSL? No way! */
#include <CommonCrypto/CommonDigest.h>
#include <unistd.h>
#include <regex.h>
#include <libgen.h>
#include <sys/stat.h>

#if LICENSE != LICENSE_NONFREE
#ifdef SECRET
#undef SECRET
#endif
/* Yes, the future non-free licensed Battman will own different secret.
 * It does not meant you have to purchase for Battman, but necessary to
 * ensure you have actually read and agreeed the Term of Use. */
#define SECRET "BattmanFree"
#endif

#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#endif

#define TOKEN_KEY @"com.torrekie.Battman.termsAccepted"
#define TOKEN_FILENAME "tou.token"

#ifndef USE_FOUNDATION
// To avoid possible sandbox restrictions against fopen/fread/fwrite/fclose, we use Foundation methods by default
#define USE_FOUNDATION 1
#endif

/* This is not a useless design, don't doubt it */
bool checked_license = false;

static void get_token(char out[65]) {
    char local_id[256] = {0};
#if !TARGET_OS_OSX
    /* On Embedded, we grant access for Vendor ID */
    sprintf(local_id, "%s", [[[UIDevice currentDevice] identifierForVendor].UUIDString UTF8String]);
#else
    /* On macOS or whatever, grant access for host */
    gethostname(local_id, sizeof(local_id));
#endif

    char input[512];
    snprintf(input, sizeof(input), "%s:%s", SECRET, local_id);

    unsigned char hash[CC_SHA256_DIGEST_LENGTH];
    CC_SHA256(input, (CC_LONG)strlen(input), hash);

    for (int i = 0; i < CC_SHA256_DIGEST_LENGTH; ++i)
        sprintf(out + i * 2, "%02x", hash[i]);

    out[64] = '\0';
}

// if unsandboxed, the config will be located at ~/Library/Preferences/com.torrekie.Battman.plist
// which editable by `defaults` command
static bool defaults_license_get(char token_out[65]) {
    [[NSUserDefaults standardUserDefaults] synchronize];
    NSString *stored = [[NSUserDefaults standardUserDefaults] stringForKey:TOKEN_KEY];
    if (!stored || [stored length] != 64)
        return false;

    strncpy(token_out, [stored UTF8String], 64);
    token_out[64] = '\0';
    return true;
}

static bool defaults_license_set(const char token[65]) {
    NSString *str = [[NSString alloc] initWithBytes:token length:64 encoding:NSUTF8StringEncoding];
    if (!str) return false;
    [[NSUserDefaults standardUserDefaults] setObject:str forKey:TOKEN_KEY];
    [[NSUserDefaults standardUserDefaults] synchronize];
    return true;
}

static char *get_token_file_path(void) {
    static char path[PATH_MAX];
    const char *home = getenv("HOME");
    if (!home) return NULL;

    memset(path, 0, PATH_MAX);
    if (match_regex(home, IOS_CONTAINER_FMT)) {
        /* should only write in Documents or Library or tmp */
        snprintf(path, sizeof(path), "%s/Library/%s", home, TOKEN_FILENAME);
    } else if (match_regex(home, MAC_CONTAINER_FMT)) {
        /* Anywhere inside Data is ok */
        snprintf(path, sizeof(path), "%s/%s", home, TOKEN_FILENAME);
    } else {
        /* If something else, then we are not sandboxed */
        snprintf(path, sizeof(path), "%s/.config/Battman/%s", home, TOKEN_FILENAME);
    }
    return path;
}

static void ensure_config_dir_exists(void) {
    const char *home = getenv("HOME");
    if (!home) return;

    NSError *error = nil;
    BOOL success = [[NSFileManager defaultManager] createDirectoryAtPath:[NSString stringWithFormat:@"%s", dirname(get_token_file_path())] withIntermediateDirectories:YES attributes:nil error:&error];

    if (!success)
        show_alert(_C("Failed to create directories"), [[error localizedDescription] UTF8String], _C("OK"));
}

bool has_accepted_terms(void) {
    char expected[65];
    get_token(expected);

    checked_license = true;

    char stored[65] = {0};

    if (defaults_license_get(stored)) {
        return strncmp(stored, expected, 64) == 0;
    }

    // fallback to file
    char *cpath = get_token_file_path();
#if !USE_FOUNDATION
    FILE *f = fopen(cpath, "r");
    if (!f) return false;

    if (fread(stored, 1, 64, f) != 64) {
        fclose(f);
        return false;
    }
    fclose(f);
#else
    @autoreleasepool {
        NSString *path = [NSString stringWithUTF8String:cpath];
        NSData *data = [NSData dataWithContentsOfFile:path];
        if (!data || [data length] < 64) {
            return false;
        }

        memcpy(stored, [data bytes], 64);
        stored[64] = '\0';
    }
#endif

    return (strncmp(stored, expected, 64) == 0);
}

void save_terms_acceptance(void) {
    char token[65];
    get_token(token);

    if (!checked_license) {
        DBGLOG(@"save_terms_acceptance called too early!");
        return;
    }

    // Compile it into binary, to warn all hackers trying to skip license check
    static const __unused char *disclaimer = "To hackers: Battman is not a paid software, please do not hook to skip license check. According to the LICENSE, once you have entered the main interface, you are deemed to have fully read, understood, and irrevocably agreed to all terms of LICENSE.";

    if (defaults_license_set(token)) return;

    // fallback to file
    ensure_config_dir_exists();

    char *cpath = get_token_file_path();
    if (!cpath)
        return;
#if !USE_FOUNDATION
    char *cpath = get_token_file_path();
    FILE *f = fopen(cpath, "w");
    if (!f) {
        show_alert(_C("Failed to write file"), strerror(errno), _C("OK"));
        return;
    }
    fwrite(token, 1, 64, f);
    fclose(f);
#else
    @autoreleasepool {
        NSString *path = [NSString stringWithUTF8String:cpath];
        NSData *data = [NSData dataWithBytes:token length:64];
        NSError *error = nil;

        BOOL success = [data writeToFile:path options:NSDataWritingAtomic error:&error];
        if (!success) {
            show_alert(_C("Failed to write file"), [[error localizedDescription] UTF8String], _C("OK"));
        }
    }
#endif
}
