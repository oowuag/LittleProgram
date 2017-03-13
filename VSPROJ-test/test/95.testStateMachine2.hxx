#include <stdio.h>

#define SINGLE_STATE_MAX_EVENT 10
typedef int FSM_EVENT_ID;
typedef struct event_param_st
{
    FSM_EVENT_ID id;
    union{
        int i;
    }data;
}FSM_EVENT;

typedef int FSM_STATE_ID;
typedef void (*FSM_FUNC)(FSM_STATE_ID id, FSM_EVENT *);
typedef struct state_event_st
{
    FSM_FUNC func;
    FSM_EVENT_ID event;
    FSM_STATE_ID state;
}FSM_STATE_EVENT;

typedef struct state_st
{
    FSM_STATE_ID id;
    char *name;
    FSM_FUNC enter_func;
    FSM_FUNC exit_func;
    FSM_FUNC default_func;
    FSM_STATE_EVENT event_table[SINGLE_STATE_MAX_EVENT]; 
}FSM_STATE;

typedef FSM_STATE STATE_TABLE[];
typedef FSM_STATE * PTR_STATE_TABLE;
#define END_EVENT_ID -1
#define END_STATE_ID -1

#define BEGIN_FSM_STATE_TABLE(state_stable) static STATE_TABLE state_stable={
#define BEGIN_STATE(id, name, enter_func, exit_func, default_func) {id, name, enter_func, exit_func, default_func,{
#define STATE_EVENT_ITEM(func, event, state) {func, event, state},
#define END_STATE(id) {NULL, END_EVENT_ID, END_STATE_ID}}},
#define END_FSM_STATE_TABLE(state_stable) {END_STATE_ID, NULL, NULL, NULL, NULL, NULL}};

typedef struct fsm_st
{
    FSM_STATE_ID state_id;
    FSM_FUNC default_func;
    PTR_STATE_TABLE state_tables;
    
}FSM;

void fsm_do_event(FSM &fsm, FSM_EVENT &event)
{
    FSM_STATE *state=&(fsm.state_tables[fsm.state_id]);
    int i=0;
    while(state->event_table[i].event!=END_EVENT_ID)
    {
        if(state->event_table[i].event==event.id)
            break;
        i++;
    }
    if(state->event_table[i].event!=END_EVENT_ID)
    {
        if(state->id!=state->event_table[i].state)
        {
            if(state->exit_func ) 
                state->exit_func(state->id, &event);
        }
        if(state->event_table[i].func)
            state->event_table[i].func(state->id, &event);

        if(state->id!=state->event_table[i].state)
        {
            if(fsm.state_tables[state->event_table[i].state].enter_func) 
                fsm.state_tables[state->event_table[i].state].enter_func(state->id, &event);
            fsm.state_id=state->event_table[i].state;
        }
    }
    else
    {
        if(state->default_func)
        {
            state->default_func(state->id, &event);
        }
        else
        {
            if(fsm.default_func)
            {
                fsm.default_func(state->id, &event);
            }
        }
    }
}

void enter_fsm(FSM_STATE_ID id, FSM_EVENT * event)
{
    printf("pre [state:%d] -> in [event:%d]\n", id, event->id);
}
void exit_fsm(FSM_STATE_ID id, FSM_EVENT * event)
{
    printf("pre [state:%d] -> out [event:%d]\n", id, event->id);
}
void defualt_fsm(FSM_STATE_ID id, FSM_EVENT * event)
{
    printf("defualt_fsm [state:%d] -> [event:%d]\n", id, event->id);
}

void func_fsm(FSM_STATE_ID id, FSM_EVENT * event)
{
    printf("exe func_fsm [%d]\n", event->id);
}

BEGIN_FSM_STATE_TABLE(my_state_table)
    BEGIN_STATE(0,"first",enter_fsm,exit_fsm,defualt_fsm)
        STATE_EVENT_ITEM(func_fsm,1,1)
        STATE_EVENT_ITEM(func_fsm,2,2)
    END_STATE(0)
    
    BEGIN_STATE(1,"second",enter_fsm,exit_fsm,defualt_fsm)
        STATE_EVENT_ITEM(func_fsm,1,2)
        STATE_EVENT_ITEM(func_fsm,2,0)
    END_STATE(1)
    
    BEGIN_STATE(2,"third",enter_fsm,exit_fsm,defualt_fsm)
        STATE_EVENT_ITEM(func_fsm,1,0)
        STATE_EVENT_ITEM(func_fsm,2,1)
    END_STATE(2)
END_FSM_STATE_TABLE(my_state_table)


int main()
{
    printf("i am main\n");
    FSM fsm={0,defualt_fsm,my_state_table};
    printf("state[%d],name[%s]\n",fsm.state_id,fsm.state_tables[fsm.state_id].name);
    FSM_EVENT event;
    event.id=1;
    event.data.i=1;
    fsm_do_event(fsm,event);
    printf("state[%d],name[%s]\n",fsm.state_id,fsm.state_tables[fsm.state_id].name);
}