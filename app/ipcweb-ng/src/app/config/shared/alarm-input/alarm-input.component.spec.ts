import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { AlarmInputComponent } from './alarm-input.component';

describe('AlarmInputComponent', () => {
  let component: AlarmInputComponent;
  let fixture: ComponentFixture<AlarmInputComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ AlarmInputComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(AlarmInputComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
