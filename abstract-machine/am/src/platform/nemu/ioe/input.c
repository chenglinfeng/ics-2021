#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  
  kbd->keydown = 0;
  kbd->keycode = AM_KEY_NONE;
  // uint32_t code = AM_KEY_NONE;
  // code = inl(KBD_ADDR);

  // kbd->keydown = (code & KEYDOWN_MASK ? true : false);
  // kbd->keycode = code & ~KEYDOWN_MASK;
  // kbd->keydown = (code & (uint32_t)KEYDOWN_MASK) != 0;
  // kbd->keycode = code & ~(uint32_t)KEYDOWN_MASK;
}
