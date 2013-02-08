#define isalnums(S) (isalnum(S) || S == '$' || S == '_')
#define MATCH_STR(S, V) (strncmp(S, V, sizeof(S) - 1) == 0)
#define MATCH_KEYWORD(S) (strncmp(S, &SOURCE[*i], sizeof(S) - 1) == 0) && !isalnums(SOURCE[*i + sizeof(S) - 1]) && ((*i += sizeof(S) - 1), 1)
#define MATCH_CHAR(C) ((SOURCE[*i] == C) && ((*i += 1), 1))
#define MATCH_TOKEN(P, L) P = &SOURCE[*i], *(L) = 0; \
  while (isalnums(SOURCE[*i])) (*i += 1), *(L) += 1;
#define REQUIRE(A) if (!(A)) return printf("Failed condition.\n"), 2;

#define LOCALS_LENGTH 0x40

int stack_length (char* locals[]) {
  int i;
  for (i = 0; i < LOCALS_LENGTH; i++) {
    if (locals[i] == NULL) {
      return i;
    }
  }
  return -1;
}

int stack_push (char* locals[], char* ptr) {
  int i;
  for (i = 0; i < LOCALS_LENGTH; i++) {
    if (locals[i] == NULL) {
      locals[i] = ptr;
      return i;
    }
  }
  return -1;
}

int stack_unshift (char* locals[], char* ptr) {
  int i;
  for (i = LOCALS_LENGTH - 1; i >= 0; i--) {
    if (locals[i] == NULL) {
      locals[i] = ptr;
      return i;
    }
  }
  return LOCALS_LENGTH - 1;
}

int stack_index (char* locals[], char* cmp) {
  int i;
  for (i = 0; i < LOCALS_LENGTH; i++) {
    if (locals[i] == NULL) {
      continue;
    }
    if (strncmp(locals[i], cmp, 256) == 0) {
      return i;
    }
  }
  return -1;
}

typedef struct global {
  char* name;
  int pos;
} global_t;

int globals_length (global_t globals[]) {
  int i;
  for (i = 0; i < 32; i++) {
    if (globals[i].name == NULL) {
      return i;
    }
  }
  return -1;
}

int globals_push (global_t globals[], global_t ptr) {
  int i;
  for (i = 0; i < 32; i++) {
    if (globals[i].name == NULL) {
      globals[i] = ptr;
      return i;
    }
  }
  return -1;
}

int globals_index (global_t globals[], char* cmp) {
  int i;
  for (i = 0; i < 32; i++) {
    if (globals[i].name == NULL) {
      return -1;
    }
    if (strncmp(globals[i].name, cmp, 256) == 0) {
      return globals[i].pos;
    }
  }
  return -1;
}

/** compile **/

typedef struct call_chain {
  uint8_t argc;
  struct call_chain *prev;
} call_chain_t;

void compile_whitespace (uint16_t *i, uint16_t totallen) {
  while (1) {
    if (isspace(SOURCE[*i])) {
      *i += 1;
    } else if (SOURCE[*i] == '/' && SOURCE[(*i) + 1] == '/') {
      while (SOURCE[*i] != '\0' && SOURCE[*i] != '\n') {
        *i += 1;
      }
    } else if (SOURCE[*i] == '/' && SOURCE[(*i) + 1] == '*') {
      while ((SOURCE[(*i)] != '*' || SOURCE[(*i) + 1] != '/') && SOURCE[*i] != '\0') {
        *i += 1;
      }
      MATCH_CHAR('*'); MATCH_CHAR('/');
    } else {
      break;
    }
  }
}

uint8_t compile_expression (uint16_t *i, uint16_t totallen, char* locals[], global_t globals[], call_chain_t *chain);

// TODO would this never return ) ?
uint8_t compile_chain (uint16_t *i, uint16_t totallen, char* locals[], global_t globals[], call_chain_t *chain, char** op)
{
  if (chain != NULL) {
    compile_whitespace(i, totallen);
    if (MATCH_CHAR(',')) {
      chain->argc++;
      return compile_expression(i, totallen, locals, globals, chain);
    } else if (MATCH_CHAR(')')) {
      chain->argc++;
      return compile_chain(i, totallen, locals, globals, chain->prev, op);
    } else {
      switch (SOURCE[*i]) {
        case '+':
        case '-':
          *op = &SOURCE[*i]; (*i)++;
          return compile_expression(i, totallen, locals, globals, chain);
      }
      return 2;
    }
  }
  return 0;
}

void compile_op (char *op) {
  switch (*op) {
    case '+': OP_ADD(); return;
    case '-': OP_SUB(); return;
  }
}

#define COMPILE_OP(V) if (V != NULL) { compile_op(V); }

uint8_t compile_expression (uint16_t *i, uint16_t totallen, char* locals[], global_t globals[], call_chain_t *chain)
{
  char *name;
  uint16_t len;
  char *op = NULL;

  while (*i <= totallen) {
    compile_whitespace(i, totallen);

    // true
    if (MATCH_KEYWORD("true")) {
      compile_chain(i, totallen, locals, globals, chain, &op);
      OP_PUSH_NUM(1);
      return 0;
    }
    // false
    if (MATCH_KEYWORD("false")) {
      compile_chain(i, totallen, locals, globals, chain, &op);
      OP_PUSH_NUM(0);
      return 0;
    }

    // Numbers
    int16_t arg;
    int consumed = 0;
    if (sscanf(&SOURCE[*i], "%" SCNd16 "%n", &arg, &consumed) != 0) {
      *i += consumed;
      if (compile_chain(i, totallen, locals, globals, chain, &op) != 0) {
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
          case '\'': t = '\''; break;
          case '\"': t = '\"'; break;
          case 'n': t = '\n'; break;
          case 't': t = '\t'; break;
          case 'r': t = '\r'; break;
          case 'b': t = 'b'; break;
          case 'f': t = 'f'; break;
          default: return 1;
        }
      }
      REQUIRE(MATCH_CHAR('\''));
      compile_chain(i, totallen, locals, globals, chain, &op);
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
        compile_whitespace(i, totallen);
        if (!MATCH_CHAR(')')) {
          if (compile_expression(i, totallen, locals, globals, &chain2) != 0) {
            return 2;
          }
        }


        uint8_t ufid;
        int globali;
        if (sscanf(name_token, "$uf%" SCNu8, &ufid) > 0) {
          OP_PUSH_NUM(chain2.argc);
          OP_USERFUNC(ufid);
        } else if ((globali = globals_index(globals, name_token)) > -1) {
          OP_CALL(globali);
          OP_POP_BUT_TOP(chain2.argc);
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
      compile_whitespace(i, totallen);
      if (MATCH_CHAR('=')) {
        if (compile_expression(i, totallen, locals, globals, chain) != 0) {
          return 2;
        }
        OP_DUP(0);
        OP_POP_LOCAL(idx);
      } else {
        compile_chain(i, totallen, locals, globals, chain, &op);
        OP_PUSH_LOCAL(idx);
      }
      COMPILE_OP(op);
      return 0;
    }

    break;
  }
  return 1;
}

uint8_t compile_statement (uint16_t *i, uint16_t totallen, char* locals[], global_t globals[])
{
  char *name;
  uint16_t len;

  while (*i <= totallen) {
    compile_whitespace(i, totallen);

    if (MATCH_CHAR(';')) {
      continue;
    }

    if (MATCH_KEYWORD("local")) {
      compile_whitespace(i, totallen);
      MATCH_TOKEN(name, &len);
      if (len == 0) {
        return 2;
      }

      compile_whitespace(i, totallen);
      REQUIRE(MATCH_CHAR(';'));

      // TODO wow this is bad
      name[len] = '\0';
      int l = stack_push(locals, name);

      // We push a 0 to the stack, so we can
      // reference this reserved value on the stack
      // directly.
      OP_PUSH_ZEROES(1);

      continue;
    }

    if (MATCH_KEYWORD("return")) {
      compile_whitespace(i, totallen);

      if (compile_expression(i, totallen, locals, globals, NULL)) {
        return 2;
      }

      compile_whitespace(i, totallen);
      REQUIRE(MATCH_CHAR(';'));

      OP_RET();

      continue;
    }

    if (MATCH_KEYWORD("while")) {
      compile_whitespace(i, totallen);
      REQUIRE(MATCH_CHAR('('));

      if (compile_expression(i, totallen, locals, globals, NULL)) {
        return 2;
      }

      int l_start = OP_LABEL();
      OP_JUNLESS_8(0x00);

      compile_whitespace(i, totallen);
      REQUIRE(MATCH_CHAR(')'));

      compile_whitespace(i, totallen);
      REQUIRE(MATCH_CHAR('{'));

      compile_statement(i, totallen, locals, globals);

      compile_whitespace(i, totallen);
      REQUIRE(MATCH_CHAR('}'));

      int l_end = OP_LABEL();
      OP_JUMP_8(l_start - l_end - 1);
      int l_tmp = OP_LABEL();
      OP_CHANGE(l_start);
      OP_JUNLESS_8(l_tmp - l_start);
      OP_CHANGE(l_tmp);

      continue;
    }

    if (MATCH_KEYWORD("if")) {
      compile_whitespace(i, totallen);
      REQUIRE(MATCH_CHAR('('));

      if (compile_expression(i, totallen, locals, globals, NULL)) {
        return 2;
      }

      int l_start = OP_LABEL();
      OP_JUNLESS_8(0x00);

      compile_whitespace(i, totallen);
      REQUIRE(MATCH_CHAR(')'));

      compile_whitespace(i, totallen);
      REQUIRE(MATCH_CHAR('{'));

      compile_statement(i, totallen, locals, globals);

      compile_whitespace(i, totallen);
      REQUIRE(MATCH_CHAR('}'));

      int l_tmp = OP_LABEL();
      OP_CHANGE(l_start);
      OP_JUNLESS_8(l_tmp - l_start);
      OP_CHANGE(l_tmp);

      continue;
    }

    // Expression
    if (compile_expression(i, totallen, locals, globals, NULL) == 0) {
      compile_whitespace(i, totallen);
      REQUIRE(MATCH_CHAR(';'));
      OP_POP();
      continue;
    }

    //printf("Unmatched char: \"%c\" (%d) at char %d\n", SOURCE[i], SOURCE[i], i);
    break;
  }
  return 1;
}

uint8_t compile_global (uint16_t *i, uint16_t totallen, int* start)
{
  global_t globals[32] = { 0 }; 

  while (*i <= totallen) {
    // Null. Terminate happily
    if (SOURCE[*i] == '\0') {
      *start = globals_index(globals, "main");
      return 0;
    }

    compile_whitespace(i, totallen);

    if (MATCH_KEYWORD("function")) {
      compile_whitespace(i, totallen);

      char* locals[LOCALS_LENGTH] = { 0 };

      // Function name
      char *name;
      uint16_t len = 0;
      MATCH_TOKEN(name, &len);

      // Parens
      compile_whitespace(i, totallen);
      REQUIRE(MATCH_CHAR('('));
      name[len] = '\0';

      compile_whitespace(i, totallen);
      if (!MATCH_CHAR(')')) {
        while (1) {
          char *arg;
          MATCH_TOKEN(arg, &len);
          if (len == 0) {
            return 2;
          }

          compile_whitespace(i, totallen);
          uint8_t findnext = MATCH_CHAR(',');
          if (!findnext) {
            REQUIRE(MATCH_CHAR(')'));
          }

          arg[len] = '\0';
          stack_unshift(locals, arg);

          if (!findnext) {
            break;
          }

          compile_whitespace(i, totallen);
        }
      }
      
      globals_push(globals, (global_t) { name, OP_LABEL() });

      // Braces
      compile_whitespace(i, totallen);
      REQUIRE(MATCH_CHAR('{'));

      compile_statement(i, totallen, locals, globals);

      REQUIRE(MATCH_CHAR('}'));
      OP_RET_VOID();

      continue;
    }

    break;
  }
  return 1;
}

int compiler (int* start) {
  vm_mem_ptr = 0;
  OP_PUSH_NUM(1);
  OP_USERFUNC(0);

  uint16_t len = strnlen(SOURCE, SOURCE_SIZE);
  uint16_t i = 0;
  int ret = compile_global(&i, len, start);
  return ret;

  int j, k, l;
  for (j = 0, k = 1, l = 1; j < i; j++, l++) {
    if (SOURCE[j] == '\n') {
      k++;
      l = 1;
    }
  }
  printf("Ended at line %d char %d.\n", k, l);
  return ret;
}