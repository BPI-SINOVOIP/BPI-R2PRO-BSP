import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { BatchInputComponent } from './batch-input.component';

describe('BatchInputComponent', () => {
  let component: BatchInputComponent;
  let fixture: ComponentFixture<BatchInputComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ BatchInputComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(BatchInputComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
