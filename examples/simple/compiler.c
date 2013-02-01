char* SOURCE = "function main() {\n\
  $uf0(1);\n\
  $uf0(1, 1);\n\
  $uf0(1, 1, 2);\n\
  $uf0(1, 1, 2, 3);\n\
  while (true) { }\n\
}";

#define isalnums(S) (isalnum(S) || S == '$')
#define MATCH_STR(S, V) (strncmp(S, V, sizeof(S) - 1) == 0)
#define MATCH_KEYWORD(S) (strncmp(S, &SOURCE[i], sizeof(S) - 1) == 0) && !isalnums(SOURCE[i + sizeof(S) - 1]) && ((i += sizeof(S) - 1), 1)
#define MATCH_CHAR(C) ((SOURCE[i] == C) && ((i += 1), 1))
#define MATCH_WHITESPACE() isspace(SOURCE[i]) && (i++, 1)
#define MATCH_TOKEN(P, L) P = &SOURCE[i], *(L) = 0; \
  while (isalnums(SOURCE[i])) i++, *(L) += 1;
#define REQUIRE(A) if (!(A)) return printf("Failed condition.\n"), i;


uint16_t compile_statement (uint16_t i, uint16_t totallen) {
  while (i < totallen) {
    if (MATCH_WHITESPACE()) {
      continue;
    }

    if (MATCH_KEYWORD("while")) {
      while (MATCH_WHITESPACE()) { }
      REQUIRE(MATCH_CHAR('('));

      while (MATCH_WHITESPACE()) { }
      if (MATCH_KEYWORD("true")) {
        OP_PUSH_U3(1);
      }

      OP_JUNLESS_8(0x04);

      while (MATCH_WHITESPACE()) { }
      REQUIRE(MATCH_CHAR(')'));

      while (MATCH_WHITESPACE()) { }
      REQUIRE(MATCH_CHAR('{'));

      while (MATCH_WHITESPACE()) { }
      REQUIRE(MATCH_CHAR('}'));

      OP_JUMP_8(0xfd);

      continue;
    }

    // Function name
    char *name;
    uint16_t len = 0;
    MATCH_TOKEN(name, &len);
    if (len > 0) {

      while (MATCH_WHITESPACE()) { }
      REQUIRE(MATCH_CHAR('('));

      /* read arguments */
      uint8_t argc = 0;
      while (SOURCE[i] != ')') {
        while (MATCH_WHITESPACE()) { }

        uint8_t arg;
        int consumed = 0;
        if (sscanf(&SOURCE[i], "%" SCNu8 "%n", &arg, &consumed) == 0) {
          break;
        }
        if (arg <= 0x7) {
          OP_PUSH_U3(arg);
        } else {
          OP_PUSH_U8(arg);
        }
        i += consumed;
        argc++;

        while (MATCH_WHITESPACE()) { }
        if (!MATCH_CHAR(',')) {
          break;
        }
      }
      // Argument count.
      OP_PUSH_U8(argc);

      REQUIRE(MATCH_CHAR(')'));

      while (MATCH_WHITESPACE()) { }
      REQUIRE(MATCH_CHAR(';'));

      uint8_t ufid;
      if (sscanf(name, "$uf%" SCNu8, &ufid) > 0) {
        OP_USERFUNC(ufid);
      }

      OP_POP();
      continue;
    }

    //printf("Unmatched char: \"%c\" (%d) at char %d\n", SOURCE[i], SOURCE[i], i);
    return i;
  }
}

uint16_t compile_global (uint16_t i, uint16_t totallen) {
  while (i < totallen) {

    if (MATCH_WHITESPACE()) {
      continue;
    }

    if (MATCH_KEYWORD("function")) {
      while (MATCH_WHITESPACE()) { }

      // Function name
      char *name;
      uint16_t len = 0;
      MATCH_TOKEN(name, &len);

      // Parens
      while (MATCH_WHITESPACE()) { }
      i++; i++;

      // Braces
      while (MATCH_WHITESPACE()) { }
      REQUIRE(MATCH_CHAR('{'));

      i = compile_statement(i, totallen);

      REQUIRE(MATCH_CHAR('}'));
      OP_RET_VOID();

      continue;
    }

    return i;
  }

  // Null. Terminate happily
  return i;
}

int compiler () {
  uint16_t len = strlen(SOURCE);
  uint16_t i = compile_global(0, len);
  return i == len;
}