

#define isalnums(S) (isalnum(S) || S == '$')
#define MATCH_STR(S, V) (strncmp(S, V, sizeof(S) - 1) == 0)
#define MATCH_KEYWORD(S) (strncmp(S, &SOURCE[*i], sizeof(S) - 1) == 0) && !isalnums(SOURCE[*i + sizeof(S) - 1]) && ((*i += sizeof(S) - 1), 1)
#define MATCH_CHAR(C) ((SOURCE[*i] == C) && ((*i += 1), 1))
#define MATCH_WHITESPACE() while (isspace(SOURCE[*i])) { *i += 1; }
#define MATCH_TOKEN(P, L) P = &SOURCE[*i], *(L) = 0; \
  while (isalnums(SOURCE[*i])) (*i += 1), *(L) += 1;
#define REQUIRE(A) if (!(A)) return printf("Failed condition.\n"), 1;

uint16_t compile_expression (uint16_t *i, uint16_t totallen)
{
  while (*i <= totallen) {
    MATCH_WHITESPACE();

    // true
    if (MATCH_KEYWORD("true")) {
      OP_PUSH_U3(1);
      return 0;
    }

    // Numbers
    uint8_t arg;
    int consumed = 0;
    if (sscanf(&SOURCE[*i], "%" SCNu8 "%n", &arg, &consumed) != 0) {
      if (arg <= 0x7) {
        OP_PUSH_U3(arg);
      } else {
        OP_PUSH_U8(arg);
      }
      *i += consumed;
      return 0;
    }

    break;
  }
  return 1;
}

int compile_statement (uint16_t *i, uint16_t totallen)
{
  while (*i <= totallen) {
    MATCH_WHITESPACE();

    if (MATCH_KEYWORD("while")) {
      MATCH_WHITESPACE();
      REQUIRE(MATCH_CHAR('('));

      if (compile_expression(i, totallen)) {
        return 1;
      }

      OP_JUNLESS_8(0x04);

      MATCH_WHITESPACE();
      REQUIRE(MATCH_CHAR(')'));

      MATCH_WHITESPACE();
      REQUIRE(MATCH_CHAR('{'));

      MATCH_WHITESPACE();
      REQUIRE(MATCH_CHAR('}'));

      OP_JUMP_8(0xfd);

      continue;
    }

    // Function name
    char *name;
    uint16_t len = 0;
    MATCH_TOKEN(name, &len);
    if (len > 0) {
      MATCH_WHITESPACE();
      REQUIRE(MATCH_CHAR('('));

      /* read arguments */
      uint8_t argc = 0;
      while (1) {
        MATCH_WHITESPACE();

        if (compile_expression(i, totallen)) {
          break;
        }
        argc++;

        MATCH_WHITESPACE();
        if (!MATCH_CHAR(',')) {
          break;
        }
      }
      // Argument count.
      OP_PUSH_U8(argc);

      REQUIRE(MATCH_CHAR(')'));

      MATCH_WHITESPACE();
      REQUIRE(MATCH_CHAR(';'));

      uint8_t ufid;
      if (sscanf(name, "$uf%" SCNu8, &ufid) > 0) {
        OP_USERFUNC(ufid);
      }

      OP_POP();
      continue;
    }

    //printf("Unmatched char: \"%c\" (%d) at char %d\n", SOURCE[i], SOURCE[i], i);
    break;
  }
  return 1;
}

int compile_global (uint16_t *i, uint16_t totallen)
{
  while (*i <= totallen) {
    // Null. Terminate happily
    if (SOURCE[*i] == '\0') {
      return 0;
    }

    MATCH_WHITESPACE();

    if (MATCH_KEYWORD("function")) {
      MATCH_WHITESPACE();

      // Function name
      char *name;
      uint16_t len = 0;
      MATCH_TOKEN(name, &len);

      // Parens
      MATCH_WHITESPACE();
      REQUIRE(MATCH_CHAR('('));
      MATCH_WHITESPACE();
      REQUIRE(MATCH_CHAR(')'));

      // Braces
      MATCH_WHITESPACE();
      REQUIRE(MATCH_CHAR('{'));

      compile_statement(i, totallen);

      REQUIRE(MATCH_CHAR('}'));
      OP_RET_VOID();

      continue;
    }

    break;
  }
  return 1;
}

int compiler () {
  uint16_t len = strnlen(SOURCE, 256);
  uint16_t i = 0;
  return compile_global(&i, len);
}