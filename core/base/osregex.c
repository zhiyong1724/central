#include "osregex.h"
#include "osstring.h"
#include "osmem.h"
#include "osstack.h"
enum
{
    MATCH = 0x00,
    SPLIT = 0xff,
};

typedef struct State
{
    char c;
    struct State *out;
    struct State *out1;
    size_t recursion;
} State;

typedef struct Fragment
{
    State *state;
    State **next;
} Fragment;

static State *newState(char c, State *out, State *out1, size_t recursion)
{
    State *state = (State *)osMalloc(sizeof(State));
    if (state != NULL)
    {
        state->c = c;
        state->out = out;
        state->out1 = out1;
        state->recursion = recursion;
        return state;
    }
    return NULL;
}

static void freeState(State *state)
{
    if (state != NULL)
    {
        if (state->out != NULL)
        {
            freeState(state->out);
        }
        if (state->out1 != NULL)
        {
            freeState(state->out1);
        }
        osFree(state);
    }
}

static void freeFragment(Fragment *fragment)
{
    freeState(fragment->state);
    fragment->state = NULL;
    fragment->next = NULL;
}

static Fragment initFragment(State *state, State *next)
{
    Fragment fragment;
    fragment.state = state;
    fragment.next = next;
    return fragment;
}

static void patch(Fragment *fragment1, Fragment *fragment2)
{
    State *state = *fragment1->next;
    State *next = NULL;
    *fragment1->next = fragment2->state;
    for (; state != NULL; state = next)
    {
        next = state->out;
        state->out = fragment2->state;
    }
}

static void append(Fragment *fragment1, Fragment *fragment2)
{
    State *state = *fragment1->next;
    if (NULL == state)
    {
        *fragment1->next = fragment2->state;
    }
    else
    {
        while (state->out != NULL)
        {
            state = state->out;
        }
        state->out = fragment2->state;
    }
}

static Fragment combineFragment(Fragment *fragment1, Fragment *fragment2)
{
    Fragment ret = initFragment(NULL, NULL);
    State *split = newState(SPLIT, fragment1->state, fragment2->state, 0);
    if (split != NULL)
    {
        append(&fragment1, &fragment2);
        ret = initFragment(split, fragment1->next);
    }
    return ret;
}

static Fragment linkFragment(Fragment *fragment1, Fragment *fragment2)
{
    patch(fragment1, fragment2);
    Fragment ret = initFragment(fragment1->state, &fragment2->next);
    return ret;
}

static Fragment combineCharByRange(char start, char end, size_t recursion)
{
    Fragment ret = initFragment(NULL, NULL);
    State *state = newState(start, NULL, NULL, recursion);
    if (state != NULL)
    {
        ret = initFragment(state, &state->out);
        start++;
        for (; start <= end; start++)
        {
            State *state = newState(start, NULL, NULL, recursion);
            if (state != NULL)
            {
                Fragment fragment = initFragment(state, &state->out);
                ret = combineFragment(&ret, &fragment);
                if (NULL == ret.state)
                {
                    freeState(state);
                    goto exception;
                }
            }
            else
            {
                goto exception;
            }
        }
    }
    goto finally;
exception:
    freeFragment(&ret);
finally:
    return ret;
}

static Fragment parseBackslash(const char *pattern, size_t recursion)
{
    Fragment ret = initFragment(NULL, NULL);
    switch (pattern[0])
    {
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
    case '*':
    case '+':
    case '?':
    case '\\':
    case '|':
    case '.':
    case '^':
    case '$':
    {
        State *state = newState(pattern[0], NULL, NULL, recursion);
        if (state != NULL)
        {
            ret = initFragment(state, &state->out);
        }
        break;
    }
    case 'w':
    {
        ret = combineCharByRange('0', '9', recursion);
        if (NULL == ret.state)
        {
            goto exception;
        }
        Fragment fragment = combineCharByRange('A', 'Z', recursion);
        if (NULL == fragment.state)
        {
            goto exception;
        }
        ret = combineFragment(&ret, &fragment);
        if (NULL == ret.state)
        {
            goto exception;
        }

        fragment = combineCharByRange('_', '_', recursion);
        if (NULL == fragment.state)
        {
            goto exception;
        }
        ret = combineFragment(&ret, &fragment);
        if (NULL == ret.state)
        {
            goto exception;
        }

        fragment = combineCharByRange('a', 'z', recursion);
        if (NULL == fragment.state)
        {
            goto exception;
        }
        ret = combineFragment(&ret, &fragment);
        if (NULL == ret.state)
        {
            goto exception;
        }
        break;
    }
    case 'W':
    {
        break;
    }
    case 's':
    {
        break;
    }
    case 'S':
    {
        break;
    }
    case 'd':
    {
        break;
    }
    case 'D':
    {
        break;
    }
    case 'b':
    {
        break;
    }
    case 'B':
    {
        break;
    }
    default:
    {
        break;
    }
    }
    goto finally;
exception:
freeFragment(&ret);
finally:
    return ret;
}

static Fragment patternToNfa(const char *pattern, size_t recursion)
{
    Fragment ret = initFragment(NULL, NULL);
    int paren = 0;
    OsStack stack;
    osStackInit(&stack, sizeof(Fragment));
    for (size_t i = 0; pattern[i] != '\0'; i++)
    {
        switch (pattern[i])
        {
        case '(':
        {
            paren++;
            Fragment fragment = patternToNfa(&pattern[i + 1], recursion + 1);
            if (fragment.state != NULL)
            {
                Fragment fragment;
                if (stack.size > 1)
                {
                    Fragment fragment2 = *((Fragment *)osStackTop(&stack));
                    osStackPop(&stack);
                    Fragment fragment1 = *((Fragment *)osStackTop(&stack));
                    osStackPop(&stack);
                    patch(&fragment1, fragment2.state);
                    fragment = initFragment(fragment1.state, fragment2.next);
                    osStackPush(&stack, &fragment);
                }
                osStackPush(&stack, &fragment);
            }
            else
            {
                goto exception;
            }
            break;
        }
        case ')':
        {
            paren--;
            break;
        }
        case '[':
        {
            break;
        }
        case ']':
        {
            break;
        }
        case '{':
        {
            break;
        }
        case '}':
        {
            break;
        }
        case '*':
        {
            break;
        }
        case '+':
        {
            break;
        }
        case '?':
        {
            break;
        }
        case '|':
        {
            break;
        }
        case '\\':
        {
            break;
        }
        case '.':
        {
            break;
        }
        case '^':
        {
            break;
        }
        case '$':
        {
            break;
        }
        default:
        {
            if (pattern[i] < SPLIT)
            {
                State *state = newState(pattern[i], NULL, NULL, recursion);
                if (state != NULL)
                {
                    Fragment fragment;
                    if (stack.size > 1)
                    {
                        Fragment fragment2 = *((Fragment *)osStackTop(&stack));
                        osStackPop(&stack);
                        Fragment fragment1 = *((Fragment *)osStackTop(&stack));
                        osStackPop(&stack);
                        patch(&fragment1, fragment2.state);
                        fragment = initFragment(fragment1.state, fragment2.next);
                        osStackPush(&stack, &fragment);
                    }
                    fragment = initFragment(state, &state->out);
                    osStackPush(&stack, &fragment);
                }
                else
                {
                    goto exception;
                }
            }
            else
            {
                goto exception;
            }
            break;
        }
        }
    }
    goto finally;
exception:
freeFragment(&ret);
finally:
    osStackFree(&stack);
    return ret;
}

void test()
{
    
}