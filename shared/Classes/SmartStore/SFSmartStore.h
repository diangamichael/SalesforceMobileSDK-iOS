/*
 Copyright (c) 2011, salesforce.com, inc. All rights reserved.
 Author: Todd Stellanova
 
 Redistribution and use of this software in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice, this list of conditions
 and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of
 conditions and the following disclaimer in the documentation and/or other materials provided
 with the distribution.
 * Neither the name of salesforce.com, inc. nor the names of its contributors may be used to
 endorse or promote products derived from this software without specific prior written
 permission of salesforce.com, inc.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#import <Foundation/Foundation.h>


/**
 The default store name used by the SFSmartStorePlugin: native code may choose
 to use separate stores.
 */
extern NSString *const kDefaultSmartStoreName;

/**
 The NSError domain for SmartStore errors.
 */
extern NSString * const kSFSmartStoreErrorDomain;

@class FMDatabase;
@class SFSoupCursor;
@class SFSoupQuerySpec;

@interface SFSmartStore : NSObject {

    //used for monitoring the status of file data protection
    BOOL    _dataProtectionKnownAvailable;
    id      _dataProtectAvailObserverToken;
    id      _dataProtectUnavailObserverToken;
    
    FMDatabase *_storeDb;
    NSString *_storeName;
    
    NSMutableDictionary *_indexSpecsBySoup;
    
}

/**
 The name of this store. 
 */
@property (nonatomic, readonly, strong) NSString *storeName;

/**
 The db access object for this store.
 */
@property (nonatomic, readonly, strong) FMDatabase *storeDb;



/**
 Use this method to obtain a shared store instance with a particular name.
 
 @param storeName The name of the store.  If in doubt, use kDefaultSmartStoreName.
 @return A shared instance of a store with the given name.
 */
+ (id)sharedStoreWithName:(NSString*)storeName;


/**
 
 You may use this method to completely remove a persistent shared store with
 the given name.
 
 @param storeName The name of the store. 
 */
+ (void)removeSharedStoreWithName:(NSString*)storeName;


/**
 @param storeName The name of the store.
 @return The filesystem diretory containing for the given store name
 */
+ (NSString *)storeDirectoryForStoreName:(NSString *)storeName;




/**
 @param storeName The name of the store (excluding paths)
 @return Does this store already exist in persistent storage (ignoring cache) ?
 */
+ (BOOL)persistentStoreExists:(NSString*)storeName;

#pragma mark - Soup manipulation methods



/**
 @param soupName the name of the soup
 @return NSArray of SFSoupIndex for the given soup
 */
- (NSArray*)indicesForSoup:(NSString*)soupName;

/**
 @param soupName the name of the soup
 @return Does a soup with the given name already exist?
 */
- (BOOL)soupExists:(NSString*)soupName;

/**
 Ensure that a soup with the given name exists.
 Either creates a new soup or returns an existing soup.
 
 @param soupName The name of the soup to register
 @param indexSpecs Array of one ore more IndexSpec objects as dictionaries
 @return YES if the soup registered OK
 */
- (BOOL)registerSoup:(NSString*)soupName withIndexSpecs:(NSArray*)indexSpecs;


/**
 Get the number of entries that would be returned with the given query spec
 
 @param soupName the name of the soup
 @param querySpec a native query spec
 */
- (NSUInteger)countEntriesInSoup:(NSString *)soupName withQuerySpec:(SFSoupQuerySpec*)querySpec;

/**
 Search soup for entries matching the querySpec

 @param soupName The name of the soup to query
 @param querySpec A querySpec as a dictionary

 @return A cursor
 */
- (SFSoupCursor*)querySoup:(NSString*)soupName withQuerySpec:(NSDictionary *)querySpec;


/**
 Search soup for entries matching the querySpec
 
 @param soupName The name of the soup to query
 @param querySpec A native SFSoupQuerySpec
 @param pageIndex The page index to start the entries at (this supports paging)
 
 @return A set of entries given the pageSize provided in the querySpec
 */
- (NSArray *)querySoup:(NSString*)soupName withQuerySpec:(SFSoupQuerySpec *)querySpec pageIndex:(NSUInteger)pageIndex;

/**
 Search soup for entries exactly matching the soup entry IDs
 
 @param soupName The name of the soup to query
 @param soupEntryIds An array of opaque soup entry IDs
 
 @return An array with zero or more entries matching the input IDs. Order is not guaranteed.
 */
- (NSArray*)retrieveEntries:(NSArray*)soupEntryIds fromSoup:(NSString*)soupName;

/**
 Insert/update entries to the soup.  Insert vs. update will be determined by the internal
 soup entry ID generated from intial entry.  If you want to specify a different identifier
 for determining existing entries, use upsertEntries:toSoup:withExternalId:
 
 @param entries The entries to insert or update.
 @param soupName The name of the soup to update.
 
 @return The array of updated entries in the soup.
 */
- (NSArray*)upsertEntries:(NSArray*)entries toSoup:(NSString*)soupName;

/**
 Insert/update entries to the soup.  Insert vs. update will be determined by the specified
 external ID argument.
 
 @param entries The entries to insert or update.
 @param soupName The name of the soup to update.
 @param externalId The user-defined query spec used to determine insert vs. update.
 
 @return The array of updated entries in the soup.
 */
- (NSArray *)upsertEntries:(NSArray *)entries toSoup:(NSString *)soupName withExternalId:(NSString *)externalId;

/**
 Remove soup entries exactly matching the soup entry IDs
 
 @param soupName The name of the soup from which to remove the soup entries
 @param soupEntryIds An array of opaque soup entry IDs from _soupEntryId
 
 */
- (void)removeEntries:(NSArray*)entryIds fromSoup:(NSString*)soupName;


/**
 Remove soup completely from the store.
 
 @param soupName The name of the soup to remove from the store.
 */
- (void)removeSoup:(NSString*)soupName;


#pragma mark - Utility methods

/**
 This is updated based on receiving notifications for
 UIApplicationProtectedDataDidBecomeAvailable / UIApplicationProtectedDataWillBecomeUnavailable.
 Note that on the simulator currently, data protection is NEVER active.
 
 @return Are we sure that file data protection (full passcode-based encryption) is available?
 */
- (BOOL)isFileDataProtectionActive;




@end
