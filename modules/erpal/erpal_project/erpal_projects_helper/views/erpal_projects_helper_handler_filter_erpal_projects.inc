<?php

/**
 * Provides views filter criteria and contextual filter 
 * "Task subtasks: Choses task and subtasks nids".
 * Filter gets nids of tasks and it's subtasks.
 *
 * Argument can work in pare with filter criteria "Task subtasks"(erpal_subtasks)
 * If erpal_subtasks value is "NO" returns single direct task id
 * If erpal_subtasks value is "YES" or doesn't exist returns direct task and 
 * subtasks
 * 
 * Filter criteria "Task subtasks"(erpal_subtasks) doesn't do any logic without
 * contestual filter "Task subtasks"
 */

/**
 * "Task subtasks" contextual filter(argument)
 * Get files of subtasks of task
 */
class erpal_projects_helper_handler_argument_erpal_projects extends views_handler_argument {

  /**
   * Clear query alter
   */
  function query($group_by = FALSE) {
    if (!$nid = $this->argument) // check arg
      return;

    if (!$node = node_load($nid)) // check in node exists
      return;

    // check in node is erpal_project or erpal_task
    if ($node->type != 'erpal_project' && $node->type != 'erpal_task')
      return;

    // prepare table
    $this->ensure_my_table();
    $alias = $this->table_alias;
    $this->query->distinct = 1; // distinct condition
    // Get filter criteria value
    // If erpal_subtasks value is "NO" get single direct task id
    if (isset($this->view->filter['erpal_subtasks']) && empty($this->view->filter['erpal_subtasks']->value[0])) {
      $this->query->add_where($alias, "{$alias}.nid", $nid);
    }
    // If erpal_subtasks value is "YES" or doesn't exist get direct task and 
    // subtasks
    else {
      // Get items of task and it's subtasks tree
      if ($node->type == 'erpal_task') {
        $child_nids = _erpal_projects_helper_get_children($nid);        
        $nids = array_unique($child_nids);
        $this->query->add_where($alias, "{$alias}.nid", $nids, "IN");
      } else

      // Get items of project and it's tasks tree
      if ($node->type == 'erpal_project') {
        $join_project_ref = new views_join();
        $join_project_ref->construct('field_data_field_project_ref', $alias, 'nid', 'entity_id');
        $joined_table = $this->query->ensure_table('field_data_field_project_ref', $this->relationship, $join_project_ref);

        $expression = "{$alias}.nid = {$nid} OR {$joined_table}.field_project_ref_target_id = {$nid}";
        $this->query->add_where_expression(0, $expression);
      }
    }
  }

}

/**
 * Provides views filter criteria and contextual filter 
 * "Task subtasks: Choses task and subtasks nids".
 * Filter gets nids of tasks and it's subtasks.
 *
 * Argument can work in pare with filter criteria "Task subtasks"(erpal_subtasks)
 * If erpal_subtasks value is "NO" returns single direct task id
 * If erpal_subtasks value is "YES" or doesn't exist returns direct task and 
 * subtasks
 * 
 * Filter criteria "Task subtasks"(erpal_subtasks) doesn't do any logic without
 * contestual filter "Task subtasks"
 */

/**
 * "Billables by projects" contextual filter(argument)
 * Get all billables of a given project
 */
class erpal_projects_helper_handler_argument_billables_by_projects extends views_handler_argument {

  /**
   * Clear query alter
   */
  function query($group_by = FALSE) {    
    if (!$nid = $this->argument) // check arg, we need a project nid
      return;

    if (!$node = node_load($nid)) // check in node exists
      return;

    // check if node is erpal_project
    if ($node->type != 'erpal_project')
      return;

    // prepare table
    $this->ensure_my_table();
    //$alias = $this->table_alias;
    $this->query->distinct = 1; // distinct condition

    $billable_ids = erpal_projects_helper_get_billables_by_subjects_for_views_handlers($nid);    
    if (!empty($billable_ids)) {
      //remove all billables that have a parent billable but add the billable_id of the parent billable instead (as this is the one that represents the revenue of all grouped sub billables.
      $a_query = db_select('billable', 'b');    
      $a_query->condition('b.parent_billable', 0, '>');
      $a_query->condition('b.billable_id', $billable_ids, 'IN');
      $a_query->fields('b', array('parent_billable', 'billable_id'));

      $result = $a_query->execute();
      $to_remove_billable_ids = array();   
      foreach ($result as $record) {
        //get the parent billable ids to add them
        $to_remove_billable_ids[] = $record->billable_id;
        $billable_ids[] = $record->parent_billable;
        //and remove the billable_ids that have a parent from the billable_ids array, we don't want to include them in the query.
      }
      
      $billable_ids = array_diff($billable_ids, $to_remove_billable_ids);
    }
    
    $this->query->add_where('billable', "billable.billable_id", $billable_ids, 'IN');
  }

}

/**
* Similar to @see erpal_projects_helper_handler_argument_billables_by_projects but this one
* The previous function returns only the parent billables, not child billables if they are grouped as the parent represents the revenue of the child.
* This handler shows only the child billables as we want to have revenue details (for contracters) and not the parent billables.
*/
class erpal_projects_helper_handler_argument_billables_by_projects_exclude_parents extends views_handler_argument {

  /**
   * Clear query alter
   */
  function query($group_by = FALSE) {
    
    if (!$nid = $this->argument) // check arg, we need a project nid
      return;

    if (!$node = node_load($nid)) // check in node exists
      return;

    // check if node is erpal_project
    if ($node->type != 'erpal_project')
      return;

    // prepare table
    $this->ensure_my_table();
    //$alias = $this->table_alias;
    $this->query->distinct = 1; // distinct condition
    
    $billable_ids = erpal_projects_helper_get_billables_by_subjects_for_views_handlers($nid);
    if (!empty($billable_ids)) {
      //remove all billables that are a parent billable but add the child billable_ids of the parent billable instead.
      $a_query = db_select('billable', 'b');    
      $a_query->condition('b.parent_billable', 0, '>');
      $a_query->condition('b.billable_id', $billable_ids, 'IN');
      $a_query->fields('b', array('parent_billable', 'billable_id'));
      $result = $a_query->execute();
      
      $parent_ids = array();
      $to_remove_billable_ids = array();   
      $parents_children = array();
      foreach ($result as $record) {
        //get the parent billable ids to add them
        $to_remove_billable_ids[] = $record->parent_billable;        
        $parent_ids[] = $record->parent_billable;   
        $parents_children[$record->parent_billable][] = $record->billable_id;
      }
      
      $billable_ids = array_diff($billable_ids, $to_remove_billable_ids);
    }

    //if the filter "Billed" is set, we remove billables that that have a parent billable that is not billed.
    //if the filter "Billed" is NOT set, we remove billables that that have a parent billable that IS billed.
    if (isset($this->view->exposed_input['billed'])) {
      $filter_billed = $this->view->exposed_input['billed'];
      
      //get all parent billables that match the billed filter
      $a_query = db_select('billable', 'b');          
      $a_query->condition('b.billable_id', $parent_ids, 'IN');
      $a_query->condition('b.billed', $filter_billed);
      $a_query->fields('b', array('parent_billable', 'billable_id'));
      $result = $a_query->execute();
      
      //now remove those billables from the result set ($billable_ids) that do have a parent billable set that is NOT in this result
      $result_billable_ids = array();

      foreach ($result as $record) {
        //here we get all parent billables that match the filter.
        $result_billable_ids[] = $parents_children[$record->billable_id];
      }
      
      //now add again those billables that don't have a parent.
      $a_query = db_select('billable', 'b');    
      $a_query->condition('b.parent_billable', 1, '!=');
      $a_query->condition('b.billable_id', $billable_ids, 'IN');
      $a_query->fields('b', array('parent_billable', 'billable_id'));
      $result = $a_query->execute();
      
      foreach ($result as $record) {
        $result_billable_ids[] = $record->billable_id;        
      }
      
      $this->query->add_where('billable', "billable.billable_id", $result_billable_ids, 'IN');
      return;
      
    }
    
    $this->query->add_where('billable', "billable.billable_id", $billable_ids, 'IN');
  }

}

/**
* Return all billables by project etc, used by views filter handlers 
* @see erpal_projects_helper_handler_argument_billables_by_projects_exclude_parents
* @see erpal_projects_helper_handler_argument_erpal_projects
*/
function erpal_projects_helper_get_billables_by_subjects_for_views_handlers($nid) {
  //get all billables of the project directly.
    $project_billables = array();
    if (!empty($nid)) {
      $project_billables = _billable_get_billables_by_subject(array($nid));
    }

    //get all billables of tasks of this project.
    $task_nids = _erpal_projects_helper_get_tasks_by_project($nid);
    $task_billables = array();
    $timetracking_nids = array();
    if (!empty($task_nids)) {
      $task_billables = _billable_get_billables_by_subject($task_nids);
      //get all billables of timetrackings of this project which have no parent task (only those are included in invoices).
      $timetracking_nids = _erpal_projects_helper_get_timetrackings_by_tasks($task_nids);
    }

    $tt_billables = array();
    if (!empty($timetracking_nids)) {
      $tt_billables = _billable_get_billables_by_subject($timetracking_nids);    
    }
     
    //get all billables of contracts of the project or task or the common activity of the project and the contract
    $contract_nids = _erpal_projects_helper_get_contracts_of_project($nid);
    $contract_billables = array();
    if (!empty($contract_nids)) {
      $contract_billables = _billable_get_billables_by_subject($contract_nids);    
    }
    
    $billable_ids = array_merge($project_billables, $task_billables, $tt_billables, $contract_billables);
    
    //get all entity IDs of a given project and add them to the where condition.
    $billable_ids = array_unique($billable_ids);
    
    return $billable_ids;
}


/**
 * "Show items of child tasks" erpal_docs_helper filter
 * Get items of project, tasks
 */
class erpal_projects_helper_handler_filter_erpal_projects extends views_handler_filter {

  /**
   * Clear query alter
   */
  function query() {
    
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['expose']['required']['#prefix'] = '<div class="element-invisible">';
    $form['expose']['required']['#value'] = TRUE;
    $form['expose']['required']['#suffix'] = '</div>';
  }

  /**
   * Subfiles checkbox. Provides show/hide choise
   */
  function value_form(&$form, &$form_state) {
    $options = array(
      '0' => 'No',
      '1' => 'Yes',
    );
    $form['value'] = array(
      '#type' => 'select',
      '#title' => t('Show elements of sub tasks'),
      '#default_value' => isset($this->value) ? $this->value : NULL,
      '#options' => $options,
    );
    return $form;
  }

}