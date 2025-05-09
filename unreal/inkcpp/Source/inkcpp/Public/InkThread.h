/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "InkVar.h"
#include "InkDelegates.h"


#include "ink/runner.h"
#include "ink/types.h"

#include "InkThread.generated.h"

class UTagList;
class AInkRuntime;
class UInkChoice;

/** Base class for all ink threads
 * @ingroup unreal
 */
UCLASS(Blueprintable)
class INKCPP_API UInkThread : public UObject
{
	GENERATED_BODY()

public:
	UInkThread();
	~UInkThread();

	// Yields the thread immediately. Will wait until Resume().
	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** Yields the thread immediately.
	 * This will stop the execution (after finishing the current line).
	 * until @ref #Resume() is called.
	 *
	 * @ref #Yield() and @ref #Resume() working with a reference counter.
	 * therefore a thread can be yield multiple times, and must then be resumed
	 * the same amount.
	 *
	 * @blueprint
	 */
	void Yield();

	UFUNCTION(BlueprintPure, Category = "Ink")
	/** Checks if the thread is stopped.
	 * @retval true if the thread currently waiting to resume
	 * @see #Yield() #Resume()
	 *
	 * @blueprint
	 */
	bool IsYielding();

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** Resumes yielded thread.
	 * @see #Yield()
	 *
	 * @blueprint
	 */
	void Resume();

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** Kills thread at next possible moment
	 *
	 * @blueprint
	 */
	void Stop();

	UFUNCTION(BlueprintPure, Category = "Ink")

	/** Access runtime the thread belongs to
	 * @return runtime of the thread
	 *
	 * @blueprint
	 */
	AInkRuntime* GetRuntime() const { return mpRuntime; }

	// Called before the thread begins executing
	UFUNCTION(BlueprintImplementableEvent, Category = "Ink")
	/** triggered after initializing the runner
	 *
	 * @blueprint
	 */
	void OnStartup();

	// Called when the thread has printed a new line
	UFUNCTION(BlueprintImplementableEvent, Category = "Ink")
	/** triggered if a new line of context is available
	 * @param line text of new line
	 * @param tags tags associated with this line
	 *
	 * @blueprint
	 */
	void OnLineWritten(const FString& line, const UTagList* tags);

	// Called when a new knot/stitch is entered (tunnels are ignored)
	UFUNCTION(BlueprintImplementableEvent, Category = "Ink")
	/** triggered if a knew knot/stitch is entered (tunneling is ignored).
	 * Triggers before the first line of a knot/stitch is written
	 * @param global_tags tags assoziated with global file
	 * @param knot_tags tags assoziated with the current knot/stitch
	 *
	 * @blueprint
	 */
	void OnKnotEntered(const UTagList* global_tags, const UTagList* knot_tags);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ink")
	/** triggered when a tag is encountered
	 * @param tag_name the tag found
	 *
	 * @blueprint
	 */
	void OnTag(const FString& tag_name);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ink")
	/** triggered when reached a choice point.
	 * @param choices possible branches to choose from, in order to continue
	 * @see #PickChoice()
	 *
	 * @blueprint
	 */
	void OnChoice(const TArray<UInkChoice*>& choices);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ink")
	/** triggered when the thread reached the end of context
	 * @see AInkRuntime::StartExisting()
	 *
	 * @blueprint
	 */
	void OnShutdown();

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** picks a choice to continue with
	 * @see UInkChoice::GetIndex()
	 * @retval false if the index is out of range
	 *
	 * @blueprint
	 */
	bool PickChoice(int index);

	// Registers a callback for a named "tag function"
	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** Register a callback for a named "tag function"
	 * @see @ref TagFunction
	 *
	 * @blueprint
	 */
	void RegisterTagFunction(FName functionName, const FTagFunctionDelegate& function);

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** register a external function.
	 * A function provides a return value
	 * @see if you do not want to return something #RegisterExternalEvent()
	 *
	 * @blueprint
	 */
	void RegisterExternalFunction(
	    const FString& functionName, const FExternalFunctionDelegate& function,
	    bool lookaheadSafe = false
	);

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** register external event.
	 * A event has the return type void.
	 * @see  If you want to return a value use #RegisterExternalFunction()
	 *
	 * @blueprint
	 */
	void RegisterExternalEvent(
	    const FString& functionName, const FExternalFunctionVoidDelegate& function,
	    bool lookaheadSafe = false
	);

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** get knots assoziated with current knot.
	 * knot tags are tags listed behind a knot `== knot name ==` before the first line of content
	 *
	 * @blueprint
	 */
	const UTagList* GetKnotTags();

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** get global tags.
	 * global tags are tags listed at the top of the file before the first line of content
	 *
	 * @blueprint
	 */
	const UTagList* GetGlobalTags();


protected:
	/** @private */
	virtual void OnStartup_Implementation() {}

	/** @private */
	virtual void OnLineWritten_Implementation(const FString& line, UTagList* tags) {}

	/** @private */
	virtual void OnTag_Implementation(const FString& line) {}

	/** @private */
	virtual void OnChoice_Implementation(const TArray<UInkChoice*>& choices) {}

	/** @private */
	virtual void OnShutdown_Implementation() {}

private:
	friend class AInkRuntime;

	void Initialize(FString path, AInkRuntime* runtime, ink::runtime::runner thread);
	bool Execute();
	bool CanExecute() const;

	bool ExecuteInternal();

	void ExecuteTagMethod(const TArray<FString>& Params);

private:
	ink::runtime::runner mpRunner;
	UTagList*            mpTags;
	UTagList*            mkTags = nullptr;
	UTagList*            mgTags = nullptr;
	TArray<UInkChoice*>  mCurrentChoices; /// @TODO: make accessible?

	TMap<FName, FTagFunctionMulticastDelegate> mTagFunctions;

	FString mStartPath;
	bool    mbHasRun;

	int  mnChoiceToChoose;
	int  mnYieldCounter;
	bool mbInChoice;
	bool mbKill;
	bool mbInitialized;
	ink::hash_t mCurrentKnot;

	UPROPERTY()
	AInkRuntime* mpRuntime;
};
