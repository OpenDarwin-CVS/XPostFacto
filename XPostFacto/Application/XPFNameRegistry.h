#include <NameRegistry.h>

#if !CALL_NOT_IN_CARBON

extern "C" {

EXTERN_API_C( OSStatus )
RegistryEntryIDInit(RegEntryID * id)                          TWOWORDINLINE(0x7000, 0xABE9);

EXTERN_API_C( OSStatus )
RegistryEntryIDDispose(RegEntryID * id)                       TWOWORDINLINE(0x7003, 0xABE9);

EXTERN_API_C( OSStatus )
RegistryEntryIterateCreate(RegEntryIter * cookie)             TWOWORDINLINE(0x7007, 0xABE9);

EXTERN_API_C( OSStatus )
RegistryEntryIterateDispose(RegEntryIter * cookie)            TWOWORDINLINE(0x7008, 0xABE9);

EXTERN_API_C( OSStatus )
RegistryCStrEntryLookup(
  const RegEntryID *       searchPointID,
  const RegCStrPathName *  pathName,
  RegEntryID *             foundEntry)                        TWOWORDINLINE(0x700C, 0xABE9);

EXTERN_API_C( OSStatus )
RegistryPropertyGetSize(
  const RegEntryID *       entryID,
  const RegPropertyName *  propertyName,
  RegPropertyValueSize *   propertySize)                      TWOWORDINLINE(0x7016, 0xABE9);

EXTERN_API_C( OSStatus )
RegistryPropertyGet(
  const RegEntryID *       entryID,
  const RegPropertyName *  propertyName,
  void *                   propertyValue,
  RegPropertyValueSize *   propertySize)                      TWOWORDINLINE(0x7017, 0xABE9);

EXTERN_API_C( OSStatus )
RegistryEntryIterate(
  RegEntryIter *        cookie,
  RegEntryIterationOp   relationship,
  RegEntryID *          foundEntry,
  Boolean *             done)                                 TWOWORDINLINE(0x700A, 0xABE9);

EXTERN_API_C( OSStatus )
RegistryEntrySearch(
  RegEntryIter *           cookie,
  RegEntryIterationOp      relationship,
  RegEntryID *             foundEntry,
  Boolean *                done,
  const RegPropertyName *  propertyName,
  const void *             propertyValue,
  RegPropertyValueSize     propertySize)                      TWOWORDINLINE(0x700B, 0xABE9);

EXTERN_API_C( OSStatus )
RegistryEntryIDCopy(
  const RegEntryID *  src,
  RegEntryID *        dst)                                    TWOWORDINLINE(0x7002, 0xABE9);

EXTERN_API_C( Boolean )
RegistryEntryIDCompare(
  const RegEntryID *  id1,
  const RegEntryID *  id2)                                    TWOWORDINLINE(0x7001, 0xABE9);

EXTERN_API_C( OSStatus )
RegistryEntryToPathSize(
  const RegEntryID *  entryID,
  RegPathNameSize *   pathSize)                               TWOWORDINLINE(0x700D, 0xABE9);

EXTERN_API_C( OSStatus )
RegistryCStrEntryToPath(
  const RegEntryID *  entryID,
  RegCStrPathName *   pathName,
  RegPathNameSize     pathSize)                               TWOWORDINLINE(0x700E, 0xABE9);

EXTERN_API_C( OSStatus )
RegistryCStrEntryToName(
  const RegEntryID *  entryID,
  RegEntryID *        parentEntry,
  RegCStrEntryName *  nameComponent,
  Boolean *           done)                                   TWOWORDINLINE(0x700F, 0xABE9);

EXTERN_API_C( OSStatus )
RegistryPropertyIterateCreate(
  const RegEntryID *  entry,
  RegPropertyIter *   cookie)                                 TWOWORDINLINE(0x7013, 0xABE9);

EXTERN_API_C( OSStatus )
RegistryPropertyIterateDispose(RegPropertyIter * cookie)      TWOWORDINLINE(0x7014, 0xABE9);

EXTERN_API_C( OSStatus )
RegistryPropertyIterate(
  RegPropertyIter *  cookie,
  RegPropertyName *  foundProperty,
  Boolean *          done)                                    TWOWORDINLINE(0x7015, 0xABE9);

EXTERN_API_C( OSStatus )
RegistryPropertySet(
  const RegEntryID *       entryID,
  const RegPropertyName *  propertyName,
  const void *             propertyValue,
  RegPropertyValueSize     propertySize)                      TWOWORDINLINE(0x7018, 0xABE9);

EXTERN_API_C( OSStatus )
RegistryPropertyGetMod(
  const RegEntryID *       entry,
  const RegPropertyName *  name,
  RegPropertyModifiers *   modifiers)                         TWOWORDINLINE(0x701B, 0xABE9);

EXTERN_API_C( OSStatus )
RegistryPropertySetMod(
  const RegEntryID *       entry,
  const RegPropertyName *  name,
  RegPropertyModifiers     modifiers)                         TWOWORDINLINE(0x701C, 0xABE9);



}  // extern "C"

#endif