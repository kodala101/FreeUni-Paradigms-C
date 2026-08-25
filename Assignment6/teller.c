#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

#include "teller.h"
#include "account.h"
#include "error.h"
#include "debug.h"
#include "branch.h"

BranchID
AccountNum_GetBranchID_(AccountNumber accountNum)
{
  return (BranchID) (accountNum >> 32);
}

/*
 * deposit money into an account
 */
int Teller_DoDeposit(Bank *bank, AccountNumber accountNum, AccountAmount amount) {
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoDeposit(account 0x%"PRIx64" amount %"PRId64")\n",
                accountNum, amount));

  Account *account = Account_LookupByNumber(bank, accountNum);

  if (account == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  BranchID branchID = AccountNum_GetBranchID_(accountNum);
  sem_wait(&(bank->branches[branchID].branchLock)); //
  sem_wait(&(account->accountLock));  //

  Account_Adjust(bank,account, amount, 1);

  sem_post(&(account->accountLock));  //
  sem_post(&(bank->branches[branchID].branchLock)); //

  return ERROR_SUCCESS;
}

/*
 * withdraw money from an account
 */
int Teller_DoWithdraw(Bank *bank, AccountNumber accountNum, AccountAmount amount) {
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoWithdraw(account 0x%"PRIx64" amount %"PRId64")\n",
                accountNum, amount));

  Account* account = Account_LookupByNumber(bank, accountNum);

  if (account == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  BranchID branchID = AccountNum_GetBranchID_(accountNum);
  sem_wait(&(bank->branches[branchID].branchLock)); //
  sem_wait(&(account->accountLock));  //

  if (amount > Account_Balance(account)) {
    sem_post(&(account->accountLock));  //
    sem_post(&(bank->branches[branchID].branchLock)); //
    return ERROR_INSUFFICIENT_FUNDS;
  }

  Account_Adjust(bank,account, -amount, 1);
  sem_post(&(account->accountLock));  //
  sem_post(&(bank->branches[branchID].branchLock)); //
  return ERROR_SUCCESS;
}

/*
 * do a tranfer from one account to another account
 */
int
Teller_DoTransfer(Bank *bank, AccountNumber srcAccountNum,
                  AccountNumber dstAccountNum,
                  AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoTransfer(src 0x%"PRIx64", dst 0x%"PRIx64
                ", amount %"PRId64")\n",
                srcAccountNum, dstAccountNum, amount));

  Account *srcAccount = Account_LookupByNumber(bank, srcAccountNum);
  if (srcAccount == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  Account *dstAccount = Account_LookupByNumber(bank, dstAccountNum);
  if (dstAccount == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  if (amount > Account_Balance(srcAccount)) {
    return ERROR_INSUFFICIENT_FUNDS;
  }

  if(srcAccountNum == dstAccountNum){
    return ERROR_SUCCESS;
  }

  /*
   * If we are doing a transfer within the branch, we tell the Account module to
   * not bother updating the branch balance since the net change for the
   * branch is 0.
   */
  int updateBranch = !Account_IsSameBranch(srcAccountNum, dstAccountNum);
  
  if (updateBranch == 0) {
     if(srcAccount->accountNumber < dstAccount->accountNumber) {   
      sem_wait(&(srcAccount->accountLock));                        
      sem_wait(&(dstAccount->accountLock));
    } else {
      sem_wait(&(dstAccount->accountLock));                          
      sem_wait(&(srcAccount->accountLock));
    }
    Account_Adjust(bank, srcAccount, -amount, updateBranch);
    Account_Adjust(bank, dstAccount, amount, updateBranch);

    sem_post(&(srcAccount->accountLock));                        
    sem_post(&(dstAccount->accountLock));
    return ERROR_SUCCESS;
  } else {
    BranchID srcBranchID = AccountNum_GetBranchID_(srcAccount->accountNumber);
    BranchID dstBranchID = AccountNum_GetBranchID_(dstAccount->accountNumber);
    if (srcBranchID < dstBranchID) {
      sem_wait(&(bank->branches[srcBranchID].branchLock));   
      sem_wait(&(bank->branches[dstBranchID].branchLock));
      sem_wait(&(srcAccount->accountLock));            
      sem_wait(&(dstAccount->accountLock));         
    } else {
      sem_wait(&(bank->branches[dstBranchID].branchLock));
      sem_wait(&(bank->branches[srcBranchID].branchLock));
      sem_wait(&(dstAccount->accountLock));
      sem_wait(&(srcAccount->accountLock));
    }
    Account_Adjust(bank, srcAccount, -amount, updateBranch);
    Account_Adjust(bank, dstAccount, amount, updateBranch);

    sem_post(&(bank->branches[srcBranchID].branchLock));   
    sem_post(&(bank->branches[dstBranchID].branchLock));
    sem_post(&(srcAccount->accountLock));            
    sem_post(&(dstAccount->accountLock));   
    return ERROR_SUCCESS;      
  }
}
