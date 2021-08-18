use smartstring::Compact;
use rhai::{ASTNode, Engine, Expr, ParseError, AST};
use std::io;
use smartstring;

fn main() -> Result<(), ParseError> {
    let engine = Engine::new();
    println!("Input a formula");
    let mut input = String::new();
    io::stdin()
        .read_line(&mut input)
        .expect("Failed to read input.");
    let ast = engine.compile_expression(&input)?;
    let all_vars = collect_all_variables(&ast);
    println!("Your input {}", input);
    println!("all vars {:#?}", all_vars);
    Ok(())
}

fn collect_all_variables(ast: &AST) -> Vec<smartstring::SmartString<Compact>>
 {
    let mut vec = Vec::new();
    ast.walk(&mut |path| -> bool {
        match path.last().expect("Always current node") {
            ASTNode::Expr(Expr::Variable(_, _, name)) => {
                let (_,_,id) = (**name).clone();
                
                vec.push(id);
                true
            }
            _ => true,
        }
    });
    vec
}
