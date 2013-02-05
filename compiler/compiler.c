#define isalnums(S) (isalnum(S) || S == '$')
#define MATCH_STR(S, V) (strncmp(S, V, sizeof(S) - 1) == 0)
#define MATCH_KEYWORD(S) (strncmp(S, &SOURCE[*i], sizeof(S) - 1) == 0) && !isalnums(SOURCE[*i + sizeof(S) - 1]) && ((*i += sizeof(S) - 1), 1)
#define MATCH_CHAR(C) ((SOURCE[*i] == C) && ((*i += 1), 1))
#define MATCH_WHITESPACE() while (isspace(SOURCE[*i])) { *i += 1; }
#define MATCH_TOKEN(P, L) P = &SOURCE[*i], *(L) = 0; \
  while (isalnums(SOURCE[*i])) (*i += 1), *(L) += 1;
#define REQUIRE(A) if (!(A)) return printf("Failed condition.\n"), 2;

int stack_length (char* locals[]) {
  int i;
  for (i = 0; i < 32; i++) {
    if (locals[i] == NULL) {
      return i;
    }
  }
  return -1;
}

int stack_push (char* locals[], char* ptr) {
  int i;
  for (i = 0; i < 32; i++) {
    if (locals[i] == NULL) {
      locals[i] = ptr;
      return i;
    }
  }
  return -1;
}

int stack_index (char* locals[], char* cmp) {
  int i;
  for (i = 0; i < 32; i++) {
    if (locals[i] == NULL) {
      return -1;
    }
    if (strncmp(locals[i], cmp, 256) == 0) {
      return i;
    }
  }
  return -1;
}

/** compile **/

typedef struct call_chain {
  uint8_t argc;
  struct call_chain *prev;
} call_chain_t;

uint8_t compile_expression (uint16_t *i, uint16_t totallen, char* locals[], call_chain_t *chain);

// TODO would this never return ) ?
uint8_t compile_chain (uint16_t *i, uint16_t totallen, char* locals[], call_chain_t *chain, char** op)
{
  if (chain != NULL) {
    MATCH_WHITESPACE();
    if (MATCH_CHAR(',')) {
      chain->argc++;
      return compile_expression(i, totallen, locals, chain);
    } else if (MATCH_CHAR(')')) {
      chain->argc++;
      return compile_chain(i, totallen, locals, chain->prev, op);
    } else {
      switch (SOURCE[*i]) {
        case '+': *op = &SOURCE[*i]; (*i)++; return compile_expression(i, totallen, locals, chain);
      }
      return 2;
    }
  }
  return 0;
}

void compile_op (char *op) {
  if (strncmp(op, "+", 1) == 0) {
    OP_PLUS();
  }
}

#define COMPILE_OP(V) if (V != NULL) { compile_op(V); }

uint8_t compile_expression (uint16_t *i, uint16_t totallen, char* locals[], call_chain_t *chain)
{
  char *name;
  uint16_t len;
  char *op = NULL;

  while (*i <= totallen) {
    MATCH_WHITESPACE();

    // true
    if (MATCH_KEYWORD("true")) {
      compile_chain(i, totallen, locals, chain, &op);
      OP_PUSH_NUM(1);
      return 0;
    }
    // false
    if (MATCH_KEYWORD("false")) {
      compile_chain(i, totallen, locals, chain, &op);
      OP_PUSH_NUM(0);
      return 0;
    }

    // Numbers
    int16_t arg;
    int consumed = 0;
    if (sscanf(&SOURCE[*i], "%" SCNd16 "%n", &arg, &consumed) != 0) {
      *i += consumed;
      if (compile_chain(i, totallen, locals, chain, &op) != 0) {
        return 1;
      }
      OP_PUSH_NUM(arg);
      COMPILE_OP(op);
      return 0;
    }

    // Characters
    if (MATCH_CHAR('\'')) {
      uint8_t t;
      t = SOURCE[(*i)++];
      if (t == '\\') {
        switch (SOURCE[(*i)++]) {
          case '\\': t = '\\'; break;
          case 'n': t = '\n'; break;
          case 't': t = '\t'; break;
          default: return 1;
        }
      }
      REQUIRE(MATCH_CHAR('\''));
      compile_chain(i, totallen, locals, chain, &op);
      OP_PUSH_U8(t);
      COMPILE_OP(op);
      return 0;
    }

    // Variables
    MATCH_TOKEN(name, &len);
    if (len != 0) {
      char name_token[256];
      strncpy(name_token, name, len);
      name_token[len] = '\0';

      // Function invocation.
      if (MATCH_CHAR('(')) {
        /* read arguments */
        call_chain_t chain2 = { 0, chain };
        MATCH_WHITESPACE();
        if (!MATCH_CHAR(')')) {
          if (compile_expression(i, totallen, locals, &chain2) != 0) {
            return 2;
          }
        }
        // Argument count.
        OP_PUSH_NUM(chain2.argc);

        uint8_t ufid;
        if (sscanf(name, "$uf%" SCNu8, &ufid) > 0) {
          OP_USERFUNC(ufid);
        } else {
          return 2;
        }

        COMPILE_OP(op);
        return 0;
      }

      int idx = stack_index(locals, name_token);
      if (idx == -1) {
        return 2;
      }
      MATCH_WHITESPACE();
      if (MATCH_CHAR('=')) {
        if (compile_expression(i, totallen, locals, chain) != 0) {
          return 2;
        }
        OP_DUP(0);
        OP_POP_LOCAL(idx);
      } else {
        compile_chain(i, totallen, locals, chain, &op);
        OP_PUSH_LOCAL(idx);
      }
      COMPILE_OP(op);
      return 0;
    }

    break;
  }
  return 1;
}

uint8_t compile_statement (uint16_t *i, uint16_t totallen, char* locals[])
{
  char *name;
  uint16_t len;

  while (*i <= totallen) {
    MATCH_WHITESPACE();

    if (MATCH_CHAR(';')) {
      continue;
    }

    if (MATCH_KEYWORD("local")) {
      MATCH_WHITESPACE();
      MATCH_TOKEN(name, &len);
      if (len == 0) {
        return 2;
      }

      MATCH_WHITESPACE();
      REQUIRE(MATCH_CHAR(';'));

      // TODO wow this is bad
      name[len] = '\0';
      int l = stack_push(locals, name);
      //OP_PUSH_ZEROES(1);
      //OP_POP_LOCAL(l);

      continue;
    }

    if (MATCH_KEYWORD("while")) {
      MATCH_WHITESPACE();
      REQUIRE(MATCH_CHAR('('));

      if (compile_expression(i, totallen, locals, NULL)) {
        return 2;
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

    // Expression
    if (compile_expression(i, totallen, locals, NULL) == 0) {
      MATCH_WHITESPACE();
      REQUIRE(MATCH_CHAR(';'));
      OP_POP();
      continue;
    }

    //printf("Unmatched char: \"%c\" (%d) at char %d\n", SOURCE[i], SOURCE[i], i);
    break;
  }
  return 1;
}

uint8_t compile_global (uint16_t *i, uint16_t totallen)
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

      char* locals[32] = { 0 };
      compile_statement(i, totallen, locals);

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