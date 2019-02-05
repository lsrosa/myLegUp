//===- GmpConv.cpp - Recreate LLVM IR from the Scop.  ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Functions for converting between gmp objects and apint.
//
//===----------------------------------------------------------------------===//
#include "polly/Support/GICHelper.h"
#include "isl/aff.h"
#include "isl/map.h"
#include "isl/schedule.h"
#include "isl/set.h"
#include "isl/union_map.h"
#include "isl/union_set.h"
#include "isl/val.h"

using namespace llvm;

__isl_give isl_val *polly::isl_valFromAPInt(isl_ctx *Ctx, const APInt Int,
                                            bool IsSigned) {
  APInt Abs;
  isl_val *v;

  if (IsSigned)
    Abs = Int.abs();
  else
    Abs = Int;

  const uint64_t *Data = Abs.getRawData();
  unsigned Words = Abs.getNumWords();

  v = isl_val_int_from_chunks(Ctx, Words, sizeof(uint64_t), Data);

  if (IsSigned && Int.isNegative())
    v = isl_val_neg(v);

  return v;
}

APInt polly::APIntFromVal(__isl_take isl_val *Val) {
  uint64_t *Data;
  int NumChunks;

  NumChunks = isl_val_n_abs_num_chunks(Val, sizeof(uint64_t));

  Data = (uint64_t *)malloc(NumChunks * sizeof(uint64_t));
  isl_val_get_abs_num_chunks(Val, sizeof(uint64_t), Data);
  APInt A(8 * sizeof(uint64_t) * NumChunks, NumChunks, Data);

  if (isl_val_is_neg(Val)) {
    A = A.zext(A.getBitWidth() + 1);
    A = -A;
  }

  if (A.getMinSignedBits() < A.getBitWidth())
    A = A.trunc(A.getMinSignedBits());

  free(Data);
  isl_val_free(Val);
  return A;
}

template <typename ISLTy, typename ISL_CTX_GETTER, typename ISL_PRINTER>
static inline std::string stringFromIslObjInternal(__isl_keep ISLTy *isl_obj,
                                                   ISL_CTX_GETTER ctx_getter_fn,
                                                   ISL_PRINTER printer_fn) {
  isl_ctx *ctx = ctx_getter_fn(isl_obj);
  isl_printer *p = isl_printer_to_str(ctx);
  printer_fn(p, isl_obj);
  char *char_str = isl_printer_get_str(p);
  std::string string(char_str);
  free(char_str);
  isl_printer_free(p);
  return string;
}

static inline isl_ctx *schedule_get_ctx(__isl_keep isl_schedule *schedule) {
  return isl_union_map_get_ctx(isl_schedule_get_map(schedule));
}

std::string polly::stringFromIslObj(__isl_keep isl_map *map) {
  return stringFromIslObjInternal(map, isl_map_get_ctx, isl_printer_print_map);
}

std::string polly::stringFromIslObj(__isl_keep isl_set *set) {
  return stringFromIslObjInternal(set, isl_set_get_ctx, isl_printer_print_set);
}

std::string polly::stringFromIslObj(__isl_keep isl_union_map *umap) {
  return stringFromIslObjInternal(umap, isl_union_map_get_ctx,
                                  isl_printer_print_union_map);
}

std::string polly::stringFromIslObj(__isl_keep isl_union_set *uset) {
  return stringFromIslObjInternal(uset, isl_union_set_get_ctx,
                                  isl_printer_print_union_set);
}

std::string polly::stringFromIslObj(__isl_keep isl_schedule *schedule) {
  return stringFromIslObjInternal(schedule, schedule_get_ctx,
                                  isl_printer_print_schedule);
}

std::string polly::stringFromIslObj(__isl_keep isl_multi_aff *maff) {
  return stringFromIslObjInternal(maff, isl_multi_aff_get_ctx,
                                  isl_printer_print_multi_aff);
}

std::string polly::stringFromIslObj(__isl_keep isl_pw_multi_aff *pma) {
  return stringFromIslObjInternal(pma, isl_pw_multi_aff_get_ctx,
                                  isl_printer_print_pw_multi_aff);
}

std::string polly::stringFromIslObj(__isl_keep isl_aff *aff) {
  return stringFromIslObjInternal(aff, isl_aff_get_ctx, isl_printer_print_aff);
}

std::string polly::stringFromIslObj(__isl_keep isl_pw_aff *pwaff) {
  return stringFromIslObjInternal(pwaff, isl_pw_aff_get_ctx,
                                  isl_printer_print_pw_aff);
}
