/**********************************************************************
 globus_oldgaa-utils.c:

Description:
        Globus-OLDGAA routines
**********************************************************************/


/**********************************************************************
                             Include header files
**********************************************************************/
#include "config.h"

#include "globus_oldgaa.h"
#include "globus_oldgaa_utils.h" 
#include "oldgaa_policy_evaluator.h"

#include "oldgaa_utils.h"
#include "config.h"

#include <string.h>	/* for strerror() */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h> 

static
int
get_default_policy_file(oldgaa_data_ptr policy_db);

static
int
oldgaa_globus_help_read_string(policy_file_context_ptr  pcontext, 
                 char                    *str, 
                 const char              *message);
static
int
oldgaa_globus_read_string (policy_file_context_ptr  pcontext,
                        char                    *str,
                        char                    **errstring);
static
int
oldgaa_globus_get_string_with_whitespaces(policy_file_context_ptr  pcontext,
                            char                    *str);
static
int
oldgaa_globus_omit_comment_line(policy_file_context_ptr  pcontext);



/**********************************************************************
                       Define module specific variables
**********************************************************************/

static  int     end_of_file;
static  uint32  m_status     = 0;


/**********************************************************************
  OLDGAA Cleanup Functions 
 **********************************************************************/

oldgaa_error_code PRIVATE
oldgaa_globus_cleanup(oldgaa_sec_context_ptr *oldgaa_sc,
                   oldgaa_rights_ptr      *rights,
                   oldgaa_options_ptr      options,
                   oldgaa_answer_ptr      *answer,  
                   oldgaa_data_ptr         policy_db, 
                   oldgaa_sec_attrb_ptr   *attributes)
{
  oldgaa_error_code oldgaa_status;
  uint32         minor_status;
        
 if(oldgaa_sc)    oldgaa_status = oldgaa_release_sec_context(&minor_status, oldgaa_sc); 
 if(rights)    oldgaa_status = oldgaa_release_rights(&minor_status, rights);   
 if(options)   oldgaa_status = oldgaa_release_options(&minor_status, options);  
 if(answer)    oldgaa_status = oldgaa_release_answer(&minor_status, answer);
 if(policy_db) oldgaa_status = oldgaa_release_data(&minor_status, policy_db);
 if(attributes)oldgaa_status = oldgaa_release_sec_attrb(&minor_status, attributes); 

 return oldgaa_status;
}



/**********************************************************************
  OLDGAA Initialization Functions 
 **********************************************************************/

oldgaa_error_code PRIVATE
oldgaa_globus_initialize(oldgaa_sec_context_ptr *oldgaa_sc,
                      oldgaa_rights_ptr      *rights,
                      oldgaa_options_ptr     *options,
                      oldgaa_data_ptr        *policy_db, 
                      char                *signer, 
                      char                *subject,
                      char                *path)
{ 
  int error=0;

 /* Allocate and fill in OLDGAA-Globus data structures */

 if(oldgaa_sc) *oldgaa_sc = oldgaa_globus_allocate_sec_context(signer); 
 if(rights) *rights = oldgaa_globus_allocate_rights();
    
 if(options) 
   {
      oldgaa_allocate_options(options);
    (*options)->value  = oldgaa_strcopy(subject, (*options)->value);  
    (*options)->length = strlen(subject); 
   }

 if(policy_db)
   {       
     oldgaa_allocate_data(policy_db);
   
     if(path) (*policy_db)->str = oldgaa_strcopy(path,(*policy_db)->str);
     else
     error = get_default_policy_file(*policy_db);
   }

   if(error)return OLDGAA_FAILURE;
   else
   return OLDGAA_SUCCESS;
}


/**********************************************************************

Function: oldgaa_globus_allocate_sec_context

Description:
	Allocates OLDGAA security context and fills in globus-specific 
        information.

Parameters:
	signer, pointer to string with name of credential signer.

Returns:
	Pointer to a oldgaa_sec_context if successful
**********************************************************************/

oldgaa_sec_context_ptr PRIVATE
oldgaa_globus_allocate_sec_context(char *signer)
{
  oldgaa_sec_context_ptr sc = NULL;
  
  oldgaa_allocate_sec_context(&sc);
  
  if(strcmp(signer, OLDGAA_ANYBODY) == 0) 
    {
      sc->identity_cred->principal->type = oldgaa_strcopy(OLDGAA_ANYBODY, 
                                    sc->identity_cred->principal->type);

      sc->identity_cred->principal->authority = oldgaa_strcopy(" ",
                                    sc->identity_cred->principal->authority);

      sc->identity_cred->principal->value = oldgaa_strcopy(" ", 
                                    sc->identity_cred->principal->value);
    }
  else
    {
     sc->identity_cred->principal->type = oldgaa_strcopy(OLDGAA_CA, 
                                    sc->identity_cred->principal->type);
   
     sc->identity_cred->principal->authority = oldgaa_strcopy(OLDGAA_X509_AUTHORITY, 
                                    sc->identity_cred->principal->authority);
    
     sc->identity_cred->principal->value = oldgaa_strcopy(signer, 
                                    sc->identity_cred->principal->value); 
    }

  return sc;
}

/**********************************************************************

Function: oldgaa_globus_allocate_rights() 

Description:
	Allocates OLDGAA rights stracture and fills in globus-specific 
        information..

Parameters:
	none

Returns:
	Pointer to a oldgaa_rights if successful
**********************************************************************/

oldgaa_rights_ptr PRIVATE
oldgaa_globus_allocate_rights()
{
 oldgaa_rights_ptr rights = NULL;

 oldgaa_allocate_rights(&rights);
 rights->reference_count++;
      
 rights->type       = oldgaa_strcopy(POSITIVE_RIGHTS,     rights->type);
 rights->authority  = oldgaa_strcopy(AUTH_GLOBUS,         rights->authority);
 rights->value      = oldgaa_strcopy(GLOBUS_RIGHTS_VALUE, rights->value);

 return rights;

}

/**********************************************************************
  Policy Retrieving Functions 
 **********************************************************************/

/**********************************************************************

Function: oldgaa_globus_policy_retrieve() 

Description:
        Upcall function for the retrieval of the object policy.

Parameters:
        object,    pointer to string with name of protected object, 
                   can be NULL
	policy_db, pointer to string with name of policy file

Returns:
	Pointer to a policy_file_context_ptr if successful
	NULL on error, setting errno.

**********************************************************************/

oldgaa_policy_ptr PRIVATE
oldgaa_globus_policy_retrieve(uint32      *minor_status,
                              UNUSED(oldgaa_data_ptr object),
			      oldgaa_data_ptr policy_db, ...)
{ 
  policy_file_context_ptr   pcontext      = NULL; 
  oldgaa_policy_ptr            policy_handle = NULL;
  int                       error_type    =  1;

  *minor_status = 0;

  pcontext = (policy_file_context_ptr)oldgaa_globus_policy_file_open(policy_db->str);

  if (pcontext)  /* parse policy */ {
    if(oldgaa_globus_parse_policy(pcontext, 
				  &policy_handle) == OLDGAA_SUCCESS) {
      return policy_handle;
    }
    else error_type = 0; 
  }

  /* error handling */
 
  if(error_type) /* policy retrieve error */ {          
    policy_db->error_code = ERROR_WHILE_RETRIEVING_POLICY;
    policy_db->error_str  = oldgaa_strcopy("error retrieving file ",
					   policy_db->error_str);
    policy_db->error_str  = strcat(policy_db->error_str, policy_db->str);       

  } 
  else          /* policy parsing error */ { 
    policy_db->error_str  = pcontext->parse_error;
    policy_db->error_str  = strcat(policy_db->error_str, pcontext->str); 

    policy_db->error_code = m_status;

  }
  
  *minor_status = m_status;

  return NULL;
}

/**********************************************************************/

static
int
get_default_policy_file(oldgaa_data_ptr policy_db)
{
  char *ca_policy_file_path  = NULL;
  char *cert_dir             = NULL;
  char *ca_policy_filename   = GRID_CA_POLICY_FILENAME;

  cert_dir = getenv("X509_CERT_DIR");

  if (cert_dir)
  {
    ca_policy_file_path = malloc(strlen(cert_dir) +
				 strlen(ca_policy_filename) +
				 2 /* for '/' and NUL */);

   if(!ca_policy_file_path) out_of_memory();

  }


  if (ca_policy_file_path)
  {
    sprintf(ca_policy_file_path, "%s/%s", cert_dir, ca_policy_filename);

    policy_db->str = oldgaa_strcopy(ca_policy_file_path, policy_db->str) ;

  }
  
 if (!ca_policy_file_path)
  {    	   
   policy_db->error_str = 
   oldgaa_strcopy("Can not find default policy location. X509_CERT_DIR is not defined.\n",
                policy_db->error_str);
   policy_db->error_code = ERROR_WHILE_GETTING_DEFAULT_POLICY_LOCATION;

   return 1;
  }
 
  return 0; 
}



/**********************************************************************

Function: oldgaa_globus_policy_file_open() 

Description:
	Open the specified policy file for reading, returning a
	context.

Parameters:
	filename, pointer to string with name of policy file

Returns:
	Pointer to a policy_file_context if successful
	NULL on error, setting errno.

**********************************************************************/

policy_file_context_ptr PRIVATE
oldgaa_globus_policy_file_open(const char *filename)
{
  char *		   open_mode = "r";
  policy_file_context_ptr  pcontext  = NULL;

  /* Check arguments */
  if (filename == NULL)
  {
    errno = ERRNO_INVALID_ARGUMENT;
    return NULL;
  }

  /* allocate and fill in pcontext structure */
  pcontext = malloc(sizeof(*pcontext));
  if (!pcontext) out_of_memory();

  pcontext->stream      = NULL;
  pcontext->parse_error = NULL;
  pcontext->str         = NULL;

  oldgaa_handle_error(&(pcontext->parse_error),"not defined");
  oldgaa_handle_error(&(pcontext->str),"not defined");


  pcontext->stream = fopen(filename, open_mode);

  if (pcontext->stream == NULL)
  {
    free(pcontext);
    return NULL;
  }
 

  return pcontext;

} /* policy_file_open() */



/**********************************************************************

Function: oldgaa_globus_policy_file_close()

Description:
	Close the policy file and deallocate memory assigned to the
	context.

Parameters:
	pcontext, pointer to the context

Returns:
	Nothing

**********************************************************************/

void PRIVATE
oldgaa_globus_policy_file_close(policy_file_context_ptr  pcontext)
{
  if (pcontext)
  {
    if(pcontext->stream) fclose(pcontext->stream);

    free(pcontext->str);
    free(pcontext->parse_error);
    free(pcontext);
  }
} 


/**********************************************************************
  Policy Parsing Functions 
 **********************************************************************/
/**********************************************************************/

static
int
oldgaa_globus_help_read_string(policy_file_context_ptr  pcontext, 
                            char                    *str, 
                            const char              *message)
{

 if (oldgaa_globus_read_string(pcontext, str, NULL)) return 1;

 if (end_of_file == TRUE) 
     {       
       oldgaa_handle_error(&(pcontext->parse_error), message);
       return 1;         
     }  
     	 
 return 0;
}



/**********************************************************************

Function: oldgaa_globus_read_string

Description:
	Read a string from a given stream up to and including the newline
	and return it in an allocated buffer.

Parameters:
	pcontext, handle to a structure, containing the stream to read from.

	str, pointer to string to be filled in.

	errstring, pointer to the pointer to be filled in with
	a pointer to a string describing an error that occurs.
	May be NULL.

Returns:
        0 on success
        1 on error

**********************************************************************/

static
int
oldgaa_globus_read_string (policy_file_context_ptr  pcontext,
                        char                    *str,
                        char                    **errstring)
{   
  if (fscanf(pcontext->stream, "%s",str) == EOF) 
  {
    end_of_file = TRUE; 
    return 0; 
  }

  oldgaa_handle_error(&(pcontext->str),str); /* set the string value to
                             report it in the case there is an error */

  
  if (str[0]== STRING_DELIMITER) /* get strings with white spaces */
  {
   if(oldgaa_globus_get_string_with_whitespaces(pcontext, str) == -1)
    {
      oldgaa_handle_error(&(pcontext->parse_error),
		        "error while reading string");      
      return 1;
    }
  }


  if (str[0]== COMMENT) /* omit comment line */
  { 
    if(oldgaa_globus_omit_comment_line(pcontext))
    {
     oldgaa_handle_error(&(pcontext->parse_error),
		       "error while reading string"); 
     return 1;
    }

    if(oldgaa_globus_read_string(pcontext, str, errstring))
    {
      
     oldgaa_handle_error(&(pcontext->parse_error),
		      "error while reading string"); 
     return 1;
    }
  } 
  
 return 0;

}

/**********************************************************************

Function: oldgaa_globus_get_string_with_whitespaces

Description:
	Read a string from a given stream up to it finds STRING_DELIMITER.

Parameters:
	pcontext, handle to a structure, containing the stream to read from.

	str, pointer to string to be filled in.

Returns:
        0 on success
       -1 on error

**********************************************************************/
static
int
oldgaa_globus_get_string_with_whitespaces(policy_file_context_ptr  pcontext,
                                       char                    *str)
{
 int  i, len = strlen(str);
 int  chr;

 for (i=0; i<len-1; i++) str[i] = str[i+1]; /* get rid of ' in the 
                                               begining of str */
 if(str[i-1] == STRING_DELIMITER) 
 {
   str[i-1] = NUL; /* clean up the tailing ' */
   return 0;
  }

                   
  while(i < MAX_STRING_SIZE) /* read chars from the stream 
                            untill see STRING_DELIMITER */
  {  
     chr = fgetc(pcontext->stream); 

     if(chr == EOF)  
    {     
      end_of_file = TRUE;

      oldgaa_handle_error(&(pcontext->parse_error),
		 "oldgaa_globus_get_string_with_white_spaces: Missing string delimiter \'");
      return -1;
    }        
      if (chr == STRING_DELIMITER) break;
      else { str[i] = chr; i++; }
  }


  if(i >= MAX_STRING_SIZE)/* string is too long */
    {
      oldgaa_handle_error(&(pcontext->parse_error),
		 "get_string_with_white_spaces: String is too long");
      return -1;
    }

  str[i] = NUL; /* terminate the string */

  return 0;
}

/**********************************************************************

Function: oldgaa_globus_omit_comment_line

Description:
	omit comment from the stream

Parameters:
	pcontext, handle to a structure, containing the stream to read from.

Returns:
        0 on success
       -1 on error

**********************************************************************/

static
int
oldgaa_globus_omit_comment_line(policy_file_context_ptr  pcontext)
{
 int chr;  
 
 while((chr = fgetc(pcontext->stream))!= EOF)
  {         
     if (chr == END_OF_LINE) break;          
  }

  if (chr == EOF) end_of_file = TRUE;

  return 0;
}   



/**********************************************************************

Function: oldgaa_globus_parse_policy() 

Description:
        Parses the policy file, filling oldgaa_policy structure. The format of
        the [olicy file is described in ?

Parameters:
        minor_status, mechanism-specific status code
        pcontext, handle to a structure, containing the stream to read from.
        policy_handle, pointer to oldgaa_policy structure to be filled in. 
                   
Returns:
	OLDGAA_SUCCESS or OLDGAA_RETREIVE_ERROR

**********************************************************************/

oldgaa_error_code  PRIVATE
oldgaa_globus_parse_policy (policy_file_context_ptr  pcontext,
                         oldgaa_policy_ptr          *policy_handle)

{
  oldgaa_conditions_ptr    all_conditions   = NULL;
/*
 *DEE all_conditions is only used in this routine to look for
 * duplicate conditions.
 */
  oldgaa_principals_ptr    start_principals = NULL;
  oldgaa_rights_ptr        start_rights     = NULL;
  oldgaa_cond_bindings_ptr cond_bind        = NULL;
  oldgaa_error_code        oldgaa_error;
  oldgaa_principals_ptr    current          = NULL;
  char                  str[MAX_STRING_SIZE] = {NUL};
  int                   cond_present     = FALSE;
  int                   new_entry        = TRUE; 

  end_of_file    = 0;
  *policy_handle = NULL;
	 
  while (!end_of_file)    
    { 
  if (new_entry == TRUE) /* start parsing new entry */
  {
   cond_present = FALSE;
   new_entry    = FALSE;

  /* get principals */
                        
    if(oldgaa_globus_parse_principals(pcontext,
                        policy_handle,
                        str,
                        &current) != OLDGAA_SUCCESS)
     { 
      oldgaa_handle_error(&(pcontext->parse_error),
		 "oldgaa_globus_parse_policy: error while parsing principal: ");
      m_status = ERROR_WHILE_PARSING_PRINCIPALS;

      goto err;
     }

    if (!start_principals)
      start_principals = current;
  }/* if (new_entry == TRUE) */

     /* continue parsing an entry */

     /* get rights */

       oldgaa_error = oldgaa_globus_parse_rights(pcontext,                              
                                           str,
                                          &start_rights,         
                                          &cond_present,
                                          &new_entry);
    if(oldgaa_error != OLDGAA_SUCCESS)
     {       
        oldgaa_handle_error(&(pcontext->parse_error),
		 "oldgaa_globus_parse_policy: error while parsing right: ");       
        m_status = ERROR_WHILE_PARSING_RIGHTS;
       goto err;

    }

 
   /* get conditions, if any */
  
   if(cond_present == TRUE)
  {
    oldgaa_error = oldgaa_globus_parse_conditions(pcontext,
                                &all_conditions,
                                 str,
                                &cond_bind,
                                &new_entry);

   if (oldgaa_error != OLDGAA_SUCCESS)               
   {
     oldgaa_handle_error(&(pcontext->parse_error),
		 "oldgaa_globus_parse_policy: error while parsing condition: ");
  
     m_status = ERROR_WHILE_PARSING_CONDITIONS; 
     goto err;

   }
  
  /* bind parsed conditions for this entry to the parsed rights 
    from this entry */

  else  oldgaa_bind_rights_to_conditions(start_rights, cond_bind);   

    /* bind parsed rights for this entry to the parsed principals 
      from this entry */

    oldgaa_bind_rights_to_principals(current, start_rights);
    oldgaa_collapse_policy(policy_handle);

 } /* if(cond_present == TRUE) */
  
 
 }/* end of while */

/* 
 * Since the conditions are now all bound to cond_bindings, 
 * we can remove the next chain pointers so they don't get
 * in the way during free if there are other chains 
 * using conditions. 
 */

/*   { */
/* 	oldgaa_conditions_ptr   c1p, c2p; */

/* 	c1p = all_conditions; */
/*     while(c1p) */
/*     { */
/* 	  c2p = c1p->next; */
/*       c1p->next = NULL; */
/* 	  c1p = c2p; */
/*     } */
/*   } */
	
  if (pcontext) oldgaa_globus_policy_file_close(pcontext);


  return OLDGAA_SUCCESS;

 err:

 oldgaa_release_principals(&m_status, policy_handle);
 oldgaa_globus_policy_file_close(pcontext);

 return OLDGAA_RETRIEVE_ERROR;
      
}

/**********************************************************************

Function: oldgaa_globus_parse_principals() 

Description:
        Parses the policy file, filling oldgaa_rincipals structure. 

Parameters:
        pcontext, handle to a structure, containing the stream to read from.
        policy, pointer to oldgaa_policy structure to be filled in. 
        tmp_str, contains a string which will be evaluated and in the end,
               new value is stored here.
        start, stores a pointer to oldgaa_principals structure of the set
        of principals which will be read by this invokation. This is needed
        by the  bind_rights_to_principals function.       
Returns:
       OLDGAA_SUCCESS or OLDGAA_PARSE_ERROR

**********************************************************************/


oldgaa_error_code PRIVATE
oldgaa_globus_parse_principals(policy_file_context_ptr  pcontext,
                            oldgaa_policy_ptr          *policy,
                            char                    *tmp_str /* IN&OUT */,
                            oldgaa_principals_ptr      *start)
{
  char               str[MAX_STRING_SIZE],*type;
  int                first     = TRUE;
  oldgaa_principals_ptr principal = NULL;

 if (*policy == NULL) /* first principal in the policy file */
  {
   if (oldgaa_globus_help_read_string(pcontext, str,"parse principals: Empty policy"))
   return OLDGAA_RETRIEVE_ERROR;   
  }
 else strcpy(str, tmp_str); /* get the value of read principal from tmp_str */

do 
  {   /* get principal's type */

   if(strcmp(str, OLDGAA_ANYBODY) == 0) /* do not check for authority and value */  
      type = OLDGAA_ANYBODY;        
   else
    if(strcmp(str,OLDGAA_USER) == 0)
       type = OLDGAA_USER;
    else 
      if(strcmp(str,OLDGAA_CA) == 0)
          type = OLDGAA_CA;
     else 
      if (strcmp(str,OLDGAA_GROUP) == 0)
          type = OLDGAA_GROUP;
       else 
         if(strcmp(str,OLDGAA_HOST) == 0)
            type = OLDGAA_HOST;
           else 
             if(strcmp(str,OLDGAA_APPLICATION) == 0)
                type = OLDGAA_APPLICATION;
             else 
             {
               oldgaa_handle_error(&(pcontext->parse_error), 
                                "parse_principals: Bad principal type");
               return OLDGAA_RETRIEVE_ERROR;
             }
          
    oldgaa_allocate_principals(&principal);

    if (type) 
    principal->type  = oldgaa_strcopy(type, principal->type);
        
    if(strcmp(type, OLDGAA_ANYBODY)== 0) /* fill in default values */
     {   
       principal->authority = oldgaa_strcopy(" ", principal->authority);
       principal->value     = oldgaa_strcopy(" ", principal->value);
     }
   else /* read defyining authority and value from the policy */
    {  
      if (oldgaa_globus_help_read_string(pcontext, str,
                         "parse_principals: Missing principal defining authority"))
      return OLDGAA_RETRIEVE_ERROR;
        
      if (str) /* expecting defining authority */ 
      principal->authority = oldgaa_strcopy(str, principal->authority);     

      if (oldgaa_globus_help_read_string(pcontext, str,
                         "parse_principals: Missing principals value"))
      return OLDGAA_RETRIEVE_ERROR;
        
      if (str) /* expecting value */ 
      principal->value = oldgaa_strcopy(str, principal->value);
       
   
   } /* end of if(type != "access_id_ANYBODY")*/ 

    if (*policy == NULL) *policy = principal;

    principal = oldgaa_add_principal(policy, principal); /* add new principal to the list */

    if(first == TRUE){ *start = principal;  first = FALSE; } 

    if (oldgaa_globus_help_read_string(pcontext, str,
                         "parse_principals: Missing rights"))
    return OLDGAA_RETRIEVE_ERROR;

    strcpy(tmp_str, str); /* return the read string */

   if( !strcmp(str,POSITIVE_RIGHTS) ||
       !strcmp(str,NEGATIVE_RIGHTS) )  /* operation set starts */  
   return OLDGAA_SUCCESS;    

  } while(!end_of_file);
   
   return  OLDGAA_SUCCESS;
}


/**********************************************************************

Function: oldgaa_globus_parse_rights() 

Description:
        Parses the policy file, filling oldgaa_rights structure. 

Parameters:
        pcontext, handle to a structure, containing the stream to read from.
        tmp_str,  contains a string which will be evaluated and in the end,
                  new value is stored here.
        start,    stores a pointer to oldgaa_rights structure of the set
                  of rights which will be read by this invokation. This is needed
                  by the  bind_rights_to_conditions function.
                  cond_present, indicates if condition set starts
        end_of_entry, indicates if new entry starts     
Returns:
       OLDGAA_SUCCESS or OLDGAA_RETRIEVE_ERROR

**********************************************************************/

oldgaa_error_code PRIVATE
oldgaa_globus_parse_rights(policy_file_context_ptr  pcontext,
                        char                    *tmp_str,
                        oldgaa_rights_ptr          *start,
                        int                     *cond_present,
                        int                     *end_of_entry)
{
  char            str[MAX_STRING_SIZE];
  int             first  = TRUE;
  oldgaa_rights_ptr  rights = NULL;
  
  strcpy(str, tmp_str); 

do{
  if( (oldgaa_strings_match(str,POSITIVE_RIGHTS) ||
       oldgaa_strings_match(str,NEGATIVE_RIGHTS))== FALSE)  /* expecting operation
                                                        set starts */
        {
          oldgaa_handle_error(&(pcontext->parse_error), "Bad right type");
          return OLDGAA_RETRIEVE_ERROR;
	}
 
  
    /* allocate fill in the oldgaa_rights structure */

    oldgaa_allocate_rights(&rights);
    if (str)
    rights->type = oldgaa_strcopy(str, rights->type);
      
    if (oldgaa_globus_help_read_string(pcontext, str,
                        "parse_rights: Missing right authority"))
    return OLDGAA_RETRIEVE_ERROR;  
 
    if (str) /* expecting defining authority */ 
    rights->authority = oldgaa_strcopy(str, rights->authority);
     
    if(oldgaa_globus_help_read_string(pcontext, str,
                        "parse_rights: Missing right value"))
    return OLDGAA_RETRIEVE_ERROR;  
  
    if (str)/* expecting value */      
    rights->value = oldgaa_strcopy(str, rights->value);
       
    if(first == TRUE){ *start = rights; first = FALSE; } 
    else oldgaa_add_rights(start, rights);

    if (oldgaa_globus_read_string(pcontext, str, NULL))     
    return OLDGAA_RETRIEVE_ERROR;
      
    strcpy(tmp_str, str); /* return the read string */
       
       if(!strncmp(str,COND_PREFIX, 5))  /* condition set starts */
       {
        *cond_present = TRUE;
         return OLDGAA_SUCCESS;   
       }
   
       if(!strncmp(str,PRINCIPAL_ACCESS_PREFIX, 6) ||
          !strncmp(str,PRINCIPAL_GRANTOR_PREFIX, 7))  /* new entry starts */        
       {
        *end_of_entry = TRUE;      
         return OLDGAA_SUCCESS; 
       }     

 } while(!end_of_file);

   return OLDGAA_SUCCESS;
}


/**********************************************************************
Function: oldgaa_globus_parse_conditions() 

Description:
        Parses the policy file, filling oldgaa_conditions structure. 

Parameters:
        pcontext,   handle to a structure, containing the stream to read from.
        conditions, pointer to oldgaa_conditions structure to be filled in. 
        tmp_str,    contains a string which will be evaluated and in the end,
                    new value is stored here.
        list,       stores a pointer to oldgaa_cond_bindings structure of the set
                    of conditions which will be read by this invokation. 
                    This is needed by the bind_rights_to_conditions function.
        end_of_entry, indicates if new entry starts     
Returns:
       OLDGAA_SUCCESS or OLDGAA_RETRIEVE_ERROR


**********************************************************************/

oldgaa_error_code PRIVATE
oldgaa_globus_parse_conditions(policy_file_context_ptr  pcontext,
                            oldgaa_conditions_ptr      *conditions,                  
                            char                    *tmp_str,
                            oldgaa_cond_bindings_ptr   *list, 
                            int                     *end_of_entry )
{
  char                  str[MAX_STRING_SIZE];
  int                   first = TRUE;
  oldgaa_conditions_ptr    cond;
  oldgaa_cond_bindings_ptr cond_bind;
  uint32        inv_minor_status = 0;

  strcpy(str, tmp_str); 
  
do
{
   if(strncmp(str,"cond_", 5) != 0)/* expecting condition set starts */
       {
          oldgaa_handle_error(&(pcontext->parse_error),"Bad condition type");
          return OLDGAA_RETRIEVE_ERROR;
       }

 /* allocate fill in the oldgaa_conditions structure */

   oldgaa_allocate_conditions(&cond);
   if (str) cond->type = oldgaa_strcopy(str,cond->type) ;
    
   if (oldgaa_globus_help_read_string(pcontext, str,
                        "parse_conditions: Missing condition authority"))
   return OLDGAA_RETRIEVE_ERROR;

   if (str) cond->authority = oldgaa_strcopy(str, cond->authority);

   if (oldgaa_globus_help_read_string(pcontext, str,
                       "parse_conditions: Missing condition value"))
   return OLDGAA_RETRIEVE_ERROR;

   if (str) cond->value = oldgaa_strcopy(str, cond->value);
    
   oldgaa_allocate_cond_bindings(&cond_bind);

   if(*conditions == NULL) { *conditions = cond; }
   cond_bind->condition =oldgaa_add_condition(conditions, cond);
   cond_bind->condition->reference_count++;

   /* If we don't add it to the list, then we need to free it! */
   if (cond_bind->condition != cond)
   {
	 cond->reference_count++; /* keep all the ducks in a row */
	 oldgaa_release_conditions(&inv_minor_status, &cond);
   }

   if(first == TRUE){ *list = cond_bind; first = FALSE; } 
   else  oldgaa_add_cond_binding(list, cond_bind);

   if (oldgaa_globus_read_string(pcontext, str, NULL))    
   return OLDGAA_RETRIEVE_ERROR;
      
   if(end_of_file == TRUE)  return  OLDGAA_SUCCESS;
    
   strcpy(tmp_str, str); /* return the read string */

    if(!strncmp(str,PRINCIPAL_ACCESS_PREFIX, 6) ||
       !strncmp(str,PRINCIPAL_GRANTOR_PREFIX, 7))  /* new entry starts */        
       {
        *end_of_entry = TRUE;      
         return OLDGAA_SUCCESS; 
       }  

    if(!strncmp(str,POS_RIGHTS_PREFIX, 3) ||
       !strncmp(str,NEG_RIGHTS_PREFIX, 3)) /* new rights set starts */          
    return OLDGAA_SUCCESS;
     

 } while (!end_of_file);

   return  OLDGAA_SUCCESS;
}


/**********************************************************************/

oldgaa_error_code PRIVATE
oldgaa_globus_get_trusted_ca_list(oldgaa_sec_attrb_ptr *attributes,
                               oldgaa_policy_ptr     policy_handle,
                               oldgaa_rights_ptr     rights)
{
  oldgaa_error_code     oldgaa_status    = OLDGAA_SUCCESS;
  uint32             minor_status  = 0;
  oldgaa_principals_ptr principal= NULL;     
   
  oldgaa_allocate_principals(&principal);
  principal->type      = oldgaa_strcopy(OLDGAA_CA,             principal->type);   
  principal->authority = oldgaa_strcopy(OLDGAA_X509_AUTHORITY, principal->authority);
  /* principal->value     = oldgaa_strcopy("\"*\"",            principal->value); */

  oldgaa_status = oldgaa_get_authorized_principals(attributes, 
                                             policy_handle,
                                             principal, 
                                             rights);

 oldgaa_status = oldgaa_release_principals(&minor_status, &principal);

 return oldgaa_status;   

}


/**********************************************************************/
