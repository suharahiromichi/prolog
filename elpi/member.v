From mathcomp Require Import all_ssreflect.

Set Implicit Arguments.
Unset Strict Implicit.
Unset Printing Implicit Defensive.

(* 
Coq の member 述語 (Inductiveに定義された Prop型）
 *)

Inductive member : nat -> seq nat -> Prop :=
| mem_hd :
    forall (n : nat) (s : seq nat), member n (cons n s)
| mem_tl :
    forall (n m : nat) (s : seq nat),  member n s -> member n (cons m s).
Hint Constructors member.

Goal member 2 (cons 1 (cons 2 (cons 2 (cons 3 nil)))).
Proof.
  info_auto.

  Undo.
  apply mem_tl.
  apply mem_hd.

  Restart.                                  (* 別解 *)
  apply mem_tl.
  apply mem_tl.
  apply mem_hd.
Qed.

(* END *)

(* 参考 *)
(* MathComp にも、こんなリストの表記方法がある。 *)

Check [:: 1; 2; 3; 4].
Check [:: 1, 2, 3, 4 & [::]].   (* 「;」でなく「,」であることに注意 *)
Check [:: 1, 2, 3 & [:: 4]].
Check [:: 1 & [:: 2 & [:: 3 & [:: 4]]]].

(* 参考。終わり。 *)
